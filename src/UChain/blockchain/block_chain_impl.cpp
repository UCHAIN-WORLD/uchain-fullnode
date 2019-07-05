/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChain/blockchain/block_chain_impl.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <algorithm>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <UChain/coin.hpp>
#include <UChainService/txs/token/token_cert.hpp>
#include <UChain/database.hpp>
#include <UChain/blockchain/block.hpp>
#include <UChain/blockchain/block_fetcher.hpp>
#include <UChain/blockchain/organizer.hpp>
#include <UChain/blockchain/settings.hpp>
#include <UChain/blockchain/tx_pool.hpp>
#include <UChain/blockchain/validate_tx_engine.hpp>
#include <UChain/blockchain/wallet_security_strategy.hpp>
namespace libbitcoin
{
namespace blockchain
{

////#define NAME "blockchain"

using namespace bc::chain;
using namespace bc::database;
using namespace boost::interprocess;
using namespace std::placeholders;
using boost::filesystem::path;

block_chain_impl::block_chain_impl(threadpool &pool,
                                   const blockchain::settings &chain_settings,
                                   const database::settings &database_settings)
    : stopped_(true),
      settings_(chain_settings),
      organizer_(pool, *this, chain_settings),
      ////read_dispatch_(pool, NAME),
      ////write_dispatch_(pool, NAME),
      tx_pool_(pool, *this, chain_settings),
      database_(database_settings)
{
}

// Close does not call stop because there is no way to detect thread join.
block_chain_impl::~block_chain_impl()
{
    close();
}

// Utilities.
// ----------------------------------------------------------------------------

static hash_list to_hashes(const block_result &result)
{
    const auto count = result.transaction_count();
    hash_list hashes;
    hashes.reserve(count);

    for (size_t index = 0; index < count; ++index)
        hashes.push_back(result.transaction_hash(index));

    return hashes;
}

// Properties.
// ----------------------------------------------------------------------------

tx_pool &block_chain_impl::pool()
{
    return tx_pool_;
}

const settings &block_chain_impl::chain_settings() const
{
    return settings_;
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start is required and the blockchain is restartable.
bool block_chain_impl::start()
{
    if (!stopped() || !database_.start())
        return false;

    stopped_ = false;
    organizer_.start();
    tx_pool_.start();

    //init the single instance here, to avoid multi-thread init confilict
    auto *temp = wallet_security_strategy::get_instance();

    return true;
}

// Stop is not required, speeds work shutdown with multiple threads.
bool block_chain_impl::stop()
{
    stopped_ = true;
    organizer_.stop();
    tx_pool_.stop();
    return database_.stop();
}

// Database threads must be joined before close is called (or destruct).
bool block_chain_impl::close()
{
    return database_.close();
}

// private
bool block_chain_impl::stopped() const
{
    // TODO: consider relying on the database stopped state.
    return stopped_;
}

// Subscriber
// ------------------------------------------------------------------------

void block_chain_impl::subscribe_reorganize(reorganize_handler handler)
{
    // Pass this through to the organizer, which issues the notifications.
    organizer_.subscribe_reorganize(handler);
}

// simple_chain (no locks, not thread safe).
// ----------------------------------------------------------------------------

bool block_chain_impl::get_gap_range(uint64_t &out_first,
                                     uint64_t &out_last) const
{
    size_t first;
    size_t last;

    if (!database_.blocks.gap_range(first, last))
        return false;

    out_first = static_cast<uint64_t>(first);
    out_last = static_cast<uint64_t>(last);
    return true;
}

bool block_chain_impl::get_next_gap(uint64_t &out_height,
                                    uint64_t start_height) const
{
    if (stopped())
        return false;

    BITCOIN_ASSERT(start_height <= bc::max_size_t);
    const auto start = static_cast<size_t>(start_height);
    size_t out;

    if (database_.blocks.next_gap(out, start))
    {
        out_height = static_cast<uint64_t>(out);
        return true;
    }

    return false;
}

bool block_chain_impl::get_difficulty(u256 &out_difficulty,
                                      uint64_t height) const
{
    size_t top;
    if (!database_.blocks.top(top))
        return false;

    out_difficulty = 0;
    for (uint64_t index = height; index <= top; ++index)
    {
        const auto bits = database_.blocks.get(index).header().timestamp;
        out_difficulty += block_work(bits);
    }

    return true;
}

bool block_chain_impl::get_header(header &out_header, uint64_t height) const
{
    auto result = database_.blocks.get(height);
    if (!result)
        return false;

    out_header = result.header();
    return true;
}

bool block_chain_impl::get_height(uint64_t &out_height,
                                  const hash_digest &block_hash) const
{
    auto result = database_.blocks.get(block_hash);
    if (!result)
        return false;

    out_height = result.height();
    return true;
}

bool block_chain_impl::get_last_height(uint64_t &out_height) const
{
    size_t top;
    if (database_.blocks.top(top))
    {
        out_height = static_cast<uint64_t>(top);
        return true;
    }

    return false;
}

bool block_chain_impl::get_outpoint_transaction(hash_digest &out_transaction,
                                                const output_point &outpoint) const
{
    const auto spend = database_.spends.get(outpoint);
    if (!spend.valid)
        return false;

    out_transaction = spend.hash;
    return true;
}

bool block_chain_impl::get_transaction(transaction &out_transaction,
                                       uint64_t &out_block_height, const hash_digest &transaction_hash) const
{
    const auto result = database_.transactions.get(transaction_hash);
    if (!result)
        return false;

    out_transaction = result.transaction();
    out_block_height = result.height();
    return true;
}

// This is safe to call concurrently (but with no other methods).
bool block_chain_impl::import(block::ptr block, uint64_t height)
{
    if (stopped())
        return false;

    // THIS IS THE DATABASE BLOCK WRITE AND INDEX OPERATION.
    database_.push(*block, height);
    return true;
}

bool block_chain_impl::push(block_info::ptr block)
{
    database_.push(*block->actual());
    return true;
}

bool block_chain_impl::pop_from(block_info::list &out_blocks,
                                uint64_t height)
{
    size_t top;

    // The chain has no genesis block, fail.
    if (!database_.blocks.top(top))
        return false;

    BITCOIN_ASSERT_MSG(top <= max_size_t - 1, "chain overflow");

    // The fork is at the top of the chain, nothing to pop.
    if (height == top + 1)
        return true;

    // The fork is disconnected from the chain, fail.
    if (height > top)
        return false;

    // If the fork is at the top there is one block to pop, and so on.
    out_blocks.reserve(top - height + 1);

    for (uint64_t index = top; index >= height; --index)
    {
        const auto block = std::make_shared<block_info>(database_.pop());
        out_blocks.push_back(block);
    }

    return true;
}

// block_chain (internal locks).
// ----------------------------------------------------------------------------

void block_chain_impl::start_write()
{
    DEBUG_ONLY(const auto result =)
    database_.begin_write();
    BITCOIN_ASSERT(result);
}

void block_chain_impl::stop_write()
{
    DEBUG_ONLY(const auto result =)
    database_.end_write();
    BITCOIN_ASSERT(result);
}

// This call is sequential, but we are preserving the callback model for now.
void block_chain_impl::store(message::block_msg::ptr block,
                             block_store_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, 0);
        return;
    }

    // We moved write to the network thread using a critical section here.
    // We do not want to give the thread to any other activity at this point.
    // A flood of valid orphans from multiple peers could tie up the CPU here,
    // but resolving that cleanly requires removing the orphan pool.

    ////write_dispatch_.ordered(
    ////    std::bind(&block_chain_impl::do_store,
    ////        this, block, handler));

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    do_store(block, handler);
    ///////////////////////////////////////////////////////////////////////////
}

// This processes the block through the organizer.
void block_chain_impl::do_store(message::block_msg::ptr block,
                                block_store_handler handler)
{
    start_write();

    // fail fast if the block is already stored...
    if (database_.blocks.get(block->header.hash()))
    {
        stop_write(handler, error::duplicate, 0);
        return;
    }

    const auto detail = std::make_shared<block_info>(block);

    // ...or if the block is already orphaned.
    if (!organizer_.add(detail))
    {
        stop_write(handler, error::duplicate, 0);
        return;
    }

    // Otherwise organize the chain...
    organizer_.organize();

    //...and then get the particular block's status.
    stop_write(handler, detail->error(), detail->height());
}

///////////////////////////////////////////////////////////////////////////////
// TODO: This should be ordered on channel distacher (modify channel queries).
// This allows channels to run concurrently with internal order preservation.
///////////////////////////////////////////////////////////////////////////////
// This performs a query in the context of the calling thread.
// The callback model is preserved currently in order to limit downstream changes.
// This change allows the caller to manage worker threads.
void block_chain_impl::fetch_serial(perform_read_functor perform_read)
{
    // Post IBD writes are ordered on the strand, so never concurrent.
    // Reads are unordered and concurrent, but effectively blocked by writes.
    const auto try_read = [this, perform_read]() {
        const auto handle = database_.begin_read();
        return (!database_.is_write_locked(handle) && perform_read(handle));
    };

    const auto do_read = [try_read]() {
        // Sleep while waiting for write to complete.
        while (!try_read())
            std::this_thread::sleep_for(asio::milliseconds(10));
    };

    // Initiate serial read operation.
    do_read();
}

////void block_chain_impl::fetch_parallel(perform_read_functor perform_read)
////{
////    // Post IBD writes are ordered on the strand, so never concurrent.
////    // Reads are unordered and concurrent, but effectively blocked by writes.
////    const auto try_read = [this, perform_read]()
////    {
////        const auto handle = database_.begin_read();
////        return (!database_.is_write_locked(handle) && perform_read(handle));
////    };
////
////    const auto do_read = [this, try_read]()
////    {
////        // Sleep while waiting for write to complete.
////        while (!try_read())
////            std::this_thread::sleep_for(std::chrono::milliseconds(10));
////    };
////
////    // Initiate async read operation.
////    read_dispatch_.concurrent(do_read);
////}

////// TODO: This should be ordered on the channel's strand, not across channels.
////void block_chain_impl::fetch_ordered(perform_read_functor perform_read)
////{
////    // Writes are ordered on the strand, so never concurrent.
////    // Reads are unordered and concurrent, but effectively blocked by writes.
////    const auto try_read = [this, perform_read]() -> bool
////    {
////        const auto handle = database_.begin_read();
////        return (!database_.is_write_locked(handle) && perform_read(handle));
////    };
////
////    const auto do_read = [this, try_read]()
////    {
////        // Sleep while waiting for write to complete.
////        while (!try_read())
////            std::this_thread::sleep_for(std::chrono::milliseconds(10));
////    };
////
////    // Initiate async read operation.
////    read_dispatch_.ordered(do_read);
////}

// block_chain (formerly fetch_ordered)
// ----------------------------------------------------------------------------

// This may generally execute 29+ queries.
// TODO: collect asynchronous calls in a function invoked directly by caller.
void block_chain_impl::fetch_block_locator(block_locator_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, handler](size_t slock) {
        hash_list locator;
        size_t top_height;
        if (!database_.blocks.top(top_height))
            return finish_fetch(slock, handler, error::operation_failed,
                                locator);

        const auto indexes = block_locator_indexes(top_height);
        for (const auto index : indexes)
        {
            hash_digest hash;
            auto found = false;
            {
                const auto result = database_.blocks.get(index);
                if (result)
                {
                    found = true;
                    hash = result.header().hash();
                }
            }
            if (!found)
                return finish_fetch(slock, handler, error::not_found, locator);

            locator.push_back(hash);
        }

        return finish_fetch(slock, handler, error::success, locator);
    };
    fetch_serial(do_fetch);
}

// Fetch start-base-stop|top+1(max 500)
// This may generally execute 502 but as many as 531+ queries.
void block_chain_impl::fetch_locator_block_hashes(
    const message::get_blocks &locator, const hash_digest &threshold,
    size_t limit, locator_block_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    // This is based on the idea that looking up by block hash to get heights
    // will be much faster than hashing each retrieved block to test for stop.
    const auto do_fetch = [this, locator, threshold, limit, handler](
                              size_t slock) {
        // Find the first block height.
        // If no start block is on our chain we start with block 0.
        size_t start = 0;
        for (const auto &hash : locator.start_hashes)
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                start = result.height();
                break;
            }
        }

        // Find the stop block height.
        // The maximum stop block is 501 blocks after start (to return 500).
        size_t stop = start + limit + 1;
        if (locator.stop_hash != null_hash)
        {
            // If the stop block is not on chain we treat it as a null stop.
            const auto stop_result = database_.blocks.get(locator.stop_hash);
            if (stop_result)
                stop = std::min(stop_result.height() + 1, stop);
        }

        // Find the threshold block height.
        // If the threshold is above the start it becomes the new start.
        if (threshold != null_hash)
        {
            const auto start_result = database_.blocks.get(threshold);
            if (start_result)
                start = std::max(start_result.height(), start);
        }

        // TODO: This largest portion can be parallelized.
        // Build the hash list until we hit last or the blockchain top.
        hash_list hashes;
        for (size_t index = start + 1; index < stop; ++index)
        {
            const auto result = database_.blocks.get(index);
            if (result)
            {
                hashes.push_back(result.header().hash());
                //                break;
            }
        }

        return finish_fetch(slock, handler, error::success, hashes);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_locator_block_headers(
    const message::get_headers &locator, const hash_digest &threshold,
    size_t limit, locator_block_headers_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    // This is based on the idea that looking up by block hash to get heights
    // will be much faster than hashing each retrieved block to test for stop.
    const auto do_fetch = [this, locator, threshold, limit, handler](
                              size_t slock) {
        // TODO: consolidate this portion with fetch_locator_block_hashes.
        //---------------------------------------------------------------------
        // Find the first block height.
        // If no start block is on our chain we start with block 0.
        size_t start = 0;
        for (const auto &hash : locator.start_hashes)
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                start = result.height();
                break;
            }
        }

        // Find the stop block height.
        // The maximum stop block is 501 blocks after start (to return 500).
        size_t stop = start + limit + 1;
        if (locator.stop_hash != null_hash)
        {
            // If the stop block is not on chain we treat it as a null stop.
            const auto stop_result = database_.blocks.get(locator.stop_hash);
            if (stop_result)
                stop = std::min(stop_result.height() + 1, stop);
        }

        // Find the threshold block height.
        // If the threshold is above the start it becomes the new start.
        if (threshold != null_hash)
        {
            const auto start_result = database_.blocks.get(threshold);
            if (start_result)
                start = std::max(start_result.height(), start);
        }
        //---------------------------------------------------------------------

        // TODO: This largest portion can be parallelized.
        // Build the hash list until we hit last or the blockchain top.
        chain::header::list headers;
        for (size_t index = start + 1; index < stop; ++index)
        {
            const auto result = database_.blocks.get(index);
            if (result)
            {
                headers.push_back(result.header());
                //                break;
            }
        }

        return finish_fetch(slock, handler, error::success, headers);
    };
    fetch_serial(do_fetch);
}

// This may execute up to 500 queries.
void block_chain_impl::filter_blocks(message::get_data::ptr message,
                                     result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto do_fetch = [this, message, handler](size_t slock) {
        auto &inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_block_type() && database_.blocks.get(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        return finish_fetch(slock, handler, error::success);
    };
    fetch_serial(do_fetch);
}

// BUGBUG: should only remove unspent transactions, other dups ok (BIP30).
void block_chain_impl::filter_transactions(message::get_data::ptr message,
                                           result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto do_fetch = [this, message, handler](size_t slock) {
        auto &inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_transaction_type() &&
                database_.transactions.get(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        return finish_fetch(slock, handler, error::success);
    };
    fetch_serial(do_fetch);
}

/// filter out block hashes that exist in the orphan pool.
void block_chain_impl::filter_orphans(message::get_data::ptr message,
                                      result_handler handler)
{
    organizer_.filter_orphans(message);
    handler(error::success);
}

// block_chain (formerly fetch_parallel)
// ------------------------------------------------------------------------

void block_chain_impl::fetch_block(uint64_t height,
                                   block_fetch_handler handler)
{
    blockchain::fetch_block(*this, height, handler);
}

void block_chain_impl::fetch_latest_transactions(uint32_t index, uint32_t count,
                                                 transactions_fetch_handler handler)
{
    uint64_t height;
    get_last_height(height);
    blockchain::fetch_latest_transactions(*this, height, index, count, handler);
}

void block_chain_impl::fetch_block(const hash_digest &hash,
                                   block_fetch_handler handler)
{
    blockchain::fetch_block(*this, hash, handler);
}

void block_chain_impl::fetch_block_header(uint64_t height,
                                          block_header_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, height, handler](size_t slock) {
        chain::header header;
        auto found = false;
        {
            const auto result = database_.blocks.get(height);
            if (result)
            {
                header = result.header();
                header.transaction_count = result.transaction_count();
                found = true;
            }
        }
        return found ? finish_fetch(slock, handler, error::success, header) : finish_fetch(slock, handler, error::not_found, chain::header());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_headers(uint64_t start,
                                           uint64_t end, bool order, locator_block_headers_fetch_handler handler, uint32_t count)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, &start, &end, order, count, handler](size_t slock) {
        chain::header header;
        std::vector<chain::header> headers;
        uint32_t tx_count = 0;
        while (start <= end)
        {
            if ((count && tx_count >= count) || !(start + 1) || !end)
                break;
            const auto result = database_.blocks.get((order ? start++ : end--));
            if (result)
            {
                chain::header header = result.header();
                header.transaction_count = result.transaction_count();
                tx_count += result.transaction_count();
                headers.push_back(header);
            }
        }

        return headers.size() ? finish_fetch(slock, handler, error::success, headers) : finish_fetch(slock, handler, error::not_found, vector<chain::header>{});
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_header(const hash_digest &hash,
                                          block_header_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock) {
        chain::header header;
        auto found = false;
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                header = result.header();
                header.transaction_count = result.transaction_count();
                found = true;
            }
        }
        return found ? finish_fetch(slock, handler, error::success, header) : finish_fetch(slock, handler, error::not_found, chain::header());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_merkle_block(uint64_t height,
                                          merkle_block_fetch_handler handler)
{
    // TODO:
    ////blockchain::fetch_merkle_block(*this, height, handler);
    handler(error::operation_failed, {});
}

void block_chain_impl::fetch_merkle_block(const hash_digest &hash,
                                          merkle_block_fetch_handler handler)
{
    // TODO:
    ////blockchain::fetch_merkle_block(*this, hash, handler);
    handler(error::operation_failed, {});
}

void block_chain_impl::fetch_block_transaction_hashes(uint64_t height,
                                                      transaction_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, height, handler](size_t slock) {
        hash_list hashes;
        auto found = false;
        {
            const auto result = database_.blocks.get(height);
            if (result)
            {
                hashes = to_hashes(result);
                found = true;
            }
        }

        return found ? finish_fetch(slock, handler, error::success, hashes) : finish_fetch(slock, handler, error::not_found, hash_list());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_transaction_hashes(const hash_digest &hash,
                                                      transaction_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock) {
        hash_list hashes;
        auto found = false;
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                hashes = to_hashes(result);
                found = true;
            }
        }

        return found ? finish_fetch(slock, handler, error::success, hashes) : finish_fetch(slock, handler, error::not_found, hash_list());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_height(const hash_digest &hash,
                                          block_height_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock) {
        std::size_t h{0};
        auto found = false;
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                h = result.height();
                found = true;
            }
        }

        return found ? finish_fetch(slock, handler, error::success, h) : finish_fetch(slock, handler, error::not_found, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_last_height(last_height_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, handler](size_t slock) {
        size_t last_height;
        return database_.blocks.top(last_height) ? finish_fetch(slock, handler, error::success, last_height) : finish_fetch(slock, handler, error::not_found, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_transaction(const hash_digest &hash,
                                         transaction_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock) {
        const auto result = database_.transactions.get(hash);
        const auto tx = result ? result.transaction() : chain::transaction();
        return result ? finish_fetch(slock, handler, error::success, tx) : finish_fetch(slock, handler, error::not_found, tx);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_transaction_index(const hash_digest &hash,
                                               transaction_index_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {}, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock) {
        const auto result = database_.transactions.get(hash);
        return result ? finish_fetch(slock, handler, error::success, result.height(),
                                     result.index())
                      : finish_fetch(slock, handler, error::not_found, 0, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_spend(const chain::output_point &outpoint,
                                   spend_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, outpoint, handler](size_t slock) {
        const auto spend = database_.spends.get(outpoint);
        const auto point = spend.valid ? chain::input_point{spend.hash, spend.index} : chain::input_point();
        return spend.valid ? finish_fetch(slock, handler, error::success, point) : finish_fetch(slock, handler, error::unspent_output, point);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_history(const bc::wallet::payment_address &address,
                                     uint64_t limit, uint64_t from_height, history_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, address, handler, limit, from_height](
                              size_t slock) {
        const auto history = database_.history.get(address.hash(), limit,
                                                   from_height);
        return finish_fetch(slock, handler, error::success, history);
    };
    fetch_serial(do_fetch);
}

bool block_chain_impl::fetch_history(const bc::wallet::payment_address &address,
                                     uint64_t limit, uint64_t from_height, history_compact::list &history)
{
    if (stopped())
    {
        return false;
    }

    boost::mutex mutex;

    mutex.lock();
    auto f = [&history, &mutex](const code &ec, const history_compact::list &history_) -> void {
        if ((code)error::success == ec)
            history = history_;
        mutex.unlock();
    };

    // Obtain payment address history from the blockchain.
    fetch_history(address, limit, from_height, f);
    boost::unique_lock<boost::mutex> lock(mutex);

    return true;
}

void block_chain_impl::fetch_stealth(const binary &filter, uint64_t from_height,
                                     stealth_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, filter, handler, from_height](
                              size_t slock) {
        const auto stealth = database_.stealth.scan(filter, from_height);
        return finish_fetch(slock, handler, error::success, stealth);
    };
    fetch_serial(do_fetch);
}

inline hash_digest block_chain_impl::get_hash(const std::string &str)
{
    data_chunk data(str.begin(), str.end());
    return sha256_hash(data);
}

inline short_hash block_chain_impl::get_short_hash(const std::string &str)
{
    data_chunk data(str.begin(), str.end());
    return ripemd160_hash(data);
}

std::shared_ptr<chain::transaction> block_chain_impl::get_spends_output(const input_point &input)
{
    const auto spend = database_.spends.get(input);
    if (spend.valid)
    {

        const auto result = database_.transactions.get(spend.hash);
        if (result)
        {
            return std::make_shared<chain::transaction>(result.transaction());
        }
    }

    return nullptr;
}

std::shared_ptr<libbitcoin::chain::wallet> block_chain_impl::is_wallet_passwd_valid(const std::string &name, const std::string &passwd)
{
    //added by chengzhiping to protect wallets from brute force password attacks.
    auto *ass = wallet_security_strategy::get_instance();
    ass->check_locked(name);

    auto wallet = get_wallet(name);
    if (wallet && wallet->get_passwd() == get_hash(passwd))
    { // wallet exist
        ass->on_auth_passwd(name, true);
        return wallet;
    }

    ass->on_auth_passwd(name, false);
    throw std::logic_error{"wallet not found or incorrect password"};
    return nullptr;
}

std::string block_chain_impl::is_wallet_lastwd_valid(const libbitcoin::chain::wallet &acc, std::string &auth, const std::string &lastwd)
{
    //added by chengzhiping to protect wallets from brute force password attacks.
    auto *ass = wallet_security_strategy::get_instance();

    std::string mnemonic;
    acc.get_mnemonic(auth, mnemonic);
    auto &&results = bc::split(mnemonic, " ", true); // with trim
    if (*results.rbegin() != lastwd)
    {
        ass->on_auth_lastwd(acc.get_name(), false);
        throw std::logic_error{"last word not matching."};
    }

    ass->on_auth_lastwd(acc.get_name(), true);

    return mnemonic;
}

void block_chain_impl::set_wallet_passwd(const std::string &name, const std::string &passwd)
{
    auto wallet = get_wallet(name);
    if (wallet)
    {
        wallet->set_passwd(passwd);
        store_wallet(wallet);
    }
    else
    {
        throw std::logic_error{"wallet not found"};
    }
}

bool block_chain_impl::is_admin_wallet(const std::string &name)
{
    auto wallet = get_wallet(name);
    if (wallet)
    {
        return wallet_priority::administrator == wallet->get_priority();
    }
    return false;
}

bool block_chain_impl::is_wallet_exist(const std::string &name)
{
    return nullptr != get_wallet(name);
}

operation_result block_chain_impl::store_wallet(std::shared_ptr<libbitcoin::chain::wallet> acc)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    if (!(acc))
    {
        throw std::runtime_error{"nullptr for wallet"};
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    database_.wallets.store(*acc);
    database_.wallets.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

std::shared_ptr<libbitcoin::chain::wallet> block_chain_impl::get_wallet(const std::string &name)
{
    return database_.wallets.get_wallet_result(get_hash(name)).get_wallet_detail();
}

/// get all the wallets in wallet database
std::shared_ptr<std::vector<libbitcoin::chain::wallet>> block_chain_impl::get_wallets()
{
    return database_.wallets.get_wallets();
}

/// delete wallet according wallet name
operation_result block_chain_impl::delete_wallet(const std::string &name)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    database_.wallets.remove(get_hash(name));
    database_.wallets.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

/// just store data into database "not do same address check"  -- todo
operation_result block_chain_impl::store_wallet_address(std::shared_ptr<wallet_address> address)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    if (!address)
    {
        throw std::runtime_error{"nullptr for address"};
    }
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    const auto hash = get_short_hash(address->get_name());
    database_.wallet_addresses.store(hash, *address);
    database_.wallet_addresses.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

/// only delete the last address of wallet
operation_result block_chain_impl::delete_wallet_address(const std::string &name)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    auto hash = get_short_hash(name);
    auto addr_vec = database_.wallet_addresses.get(hash);
    for (auto each : addr_vec)
        database_.wallet_addresses.delete_last_row(hash);
    database_.wallet_addresses.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

/// delete some of the address from the last
operation_result block_chain_impl::delete_n_wallet_address(const std::string &name, uint64_t count)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    auto hash = get_short_hash(name);
    auto addr_vec = database_.wallet_addresses.get(hash);
    if (count >= addr_vec.size())
        throw std::logic_error{"Parameter count should less than the count of your address!"};
    for (size_t i = 0; i < count; i++)
        database_.wallet_addresses.delete_last_row(hash);
    database_.wallet_addresses.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

std::shared_ptr<wallet_address> block_chain_impl::get_wallet_address(
    const std::string &name, const std::string &address)
{
    return database_.wallet_addresses.get(get_short_hash(name), address);
}

std::shared_ptr<wallet_address::list> block_chain_impl::get_wallet_addresses(
    const std::string &name)
{
    auto sp_addr = std::make_shared<wallet_address::list>();
    auto result = database_.wallet_addresses.get(get_short_hash(name));
    if (result.size())
    {
        //sp_addr = std::make_shared<wallet_address::list>();
        const auto action = [&sp_addr](const wallet_address &elem) {
            sp_addr->emplace_back(std::move(elem));
        };
        std::for_each(result.begin(), result.end(), action);
    }
    return sp_addr;
}

operation_result block_chain_impl::store_wallet_token(
    const token_detail &detail,
    const string &name)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    const auto hash = get_short_hash(name);
    database_.wallet_tokens.store(hash, detail);
    database_.wallet_tokens.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

operation_result block_chain_impl::store_wallet_token(
    std::shared_ptr<token_detail> detail,
    const string &name)
{
    if (!(detail))
    {
        throw std::runtime_error{"nullptr for token"};
    }
    return store_wallet_token(*detail, name);
}

/// delete wallet token by wallet name
operation_result block_chain_impl::delete_wallet_token(const std::string &name)
{
    if (stopped())
    {
        return operation_result::failure;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    auto hash = get_short_hash(name);
    auto token_vec = database_.wallet_tokens.get(hash);
    for (auto each : token_vec) // just use token count
        database_.wallet_tokens.delete_last_row(hash);
    database_.wallet_tokens.sync();
    ///////////////////////////////////////////////////////////////////////////
    return operation_result::okay;
}

std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_token(
    const std::string &name, const std::string &token_name, business_kind kind)
{
    auto sp_token_vec = get_wallet_tokens(name, kind);
    auto ret_vector = std::make_shared<business_address_token::list>();

    const auto action = [&](const business_address_token &addr_token) {
        if (addr_token.detail.get_symbol() == token_name)
            ret_vector->emplace_back(std::move(addr_token));
    };

    std::for_each(sp_token_vec->begin(), sp_token_vec->end(), action);

    return ret_vector;
}

std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_token(
    const std::string &name, const std::string &token_name)
{
    auto sp_token_vec = get_wallet_tokens(name);
    auto ret_vector = std::make_shared<business_address_token::list>();

    const auto action = [&](const business_address_token &addr_token) {
        if (addr_token.detail.get_symbol() == token_name)
            ret_vector->emplace_back(std::move(addr_token));
    };

    std::for_each(sp_token_vec->begin(), sp_token_vec->end(), action);

    return ret_vector;
}

static history::list expand_history(history_compact::list &compact)
{
    history::list result;
    result.reserve(compact.size());

    std::unordered_map<uint64_t, history *> map_output;
    // Process all outputs.
    for (auto output = compact.begin(); output != compact.end(); ++output)
    {
        if (output->kind == point_kind::output)
        {
            history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->value;
            row.spend = {null_hash, max_uint32};
            row.temporary_checksum = output->point.checksum();
            result.emplace_back(row);
            map_output[row.temporary_checksum] = &result.back();
        }
    }

    //process the spends.
    for (const auto &spend : compact)
    {
        if (spend.kind != point_kind::spend)
            continue;

        auto r = map_output.find(spend.previous_checksum);
        if (r != map_output.end() && r->second->spend.hash == null_hash)
        {
            r->second->spend = spend.point;
            r->second->spend_height = spend.height;
            continue;
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        {
            history row;
            row.output = {null_hash, max_uint32};
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto &row : result)
    {
        if (row.spend.hash == null_hash)
        {
            row.spend_height = max_uint64;
        }
    }

    // sort by height and index of output(unspend) and spend in order.
    // 1. unspend before spend
    // because unspend's spend height is max_uint64,
    // so sort by spend height decreasely can ensure this.
    // 2. for spend
    // spent height first, decreasely
    // spend index second, decreasely
    // 3. for unspend
    // output height first, decreasely
    // output index second, decreasely
    std::sort(result.begin(), result.end(),
              [](const history &elem1, const history &elem2) {
                  typedef std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> cmp_tuple_t;
                  cmp_tuple_t tuple1(elem1.spend_height, elem1.spend.index, elem1.output_height, elem1.output.index);
                  cmp_tuple_t tuple2(elem2.spend_height, elem2.spend.index, elem2.output_height, elem2.output.index);
                  return tuple1 > tuple2;
              });
    return result;
}

history::list block_chain_impl::get_address_history(const bc::wallet::payment_address &addr, bool add_memory_pool, uint64_t from_height)
{
    history_compact::list cmp_history;
    bool result = true;
    if (add_memory_pool)
    {
        result = get_history(addr, 0, from_height, cmp_history);
    }
    else
    {
        result = fetch_history(addr, 0, from_height, cmp_history);
    }
    if (result)
    {
        return expand_history(cmp_history);
    }
    return history::list();
}

std::shared_ptr<token_cert> block_chain_impl::get_wallet_token_cert(
    const std::string &wallet, const std::string &symbol, token_cert_type cert_type)
{
    BITCOIN_ASSERT(!symbol.empty());
    BITCOIN_ASSERT(cert_type != token_cert_ns::none);

    auto pvaddr = get_wallet_addresses(wallet);
    if (!pvaddr)
        return nullptr;

    chain::transaction tx_temp;
    uint64_t tx_height;

    for (auto &each : *pvaddr)
    {
        bc::wallet::payment_address payment_address(each.get_address());
        auto &&rows = get_address_history(payment_address);

        for (auto &row : rows)
        {
            // spend unconfirmed (or no spend attempted)
            if ((row.spend.hash == null_hash) && get_transaction(row.output.hash, tx_temp, tx_height))
            {
                BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
                const auto &output = tx_temp.outputs.at(row.output.index);
                if (output.is_token_cert())
                {
                    auto cert = output.get_token_cert();
                    if (symbol != cert.get_symbol() || cert_type != cert.get_type())
                    {
                        continue;
                    }

                    return std::make_shared<token_cert>(cert);
                }
            }
        }
    }

    return nullptr;
}

std::shared_ptr<business_address_token_cert::list>
block_chain_impl::get_address_token_certs(const std::string &address, const std::string &symbol, token_cert_type cert_type)
{
    auto ret_vector = std::make_shared<business_address_token_cert::list>();
    auto &&business_certs = database_.address_tokens.get_token_certs(address, symbol, cert_type, 0);
    for (auto &business_cert : business_certs)
    {
        ret_vector->emplace_back(std::move(business_cert));
    }
    return ret_vector;
}

std::shared_ptr<business_address_token_cert::list>
block_chain_impl::get_wallet_token_certs(const std::string &wallet, const std::string &symbol, token_cert_type cert_type)
{
    auto ret_vector = std::make_shared<business_address_token_cert::list>();
    auto pvaddr = get_wallet_addresses(wallet);
    if (pvaddr)
    {
        for (const auto &wallet_address : *pvaddr)
        {
            auto &&business_certs = database_.address_tokens.get_token_certs(wallet_address.get_address(), symbol, cert_type, 0);
            for (auto &business_cert : business_certs)
            {
                ret_vector->emplace_back(std::move(business_cert));
            }
        }
    }
    return ret_vector;
}

std::shared_ptr<token_cert::list> block_chain_impl::get_issued_token_certs()
{
    auto sp_vec = std::make_shared<token_cert::list>();
    auto sp_token_certs_vec = database_.certs.get_blockchain_token_certs();
    for (const auto &each : *sp_token_certs_vec)
        sp_vec->emplace_back(std::move(each));
    return sp_vec;
}

bool block_chain_impl::is_token_cert_exist(const std::string &symbol, token_cert_type cert_type)
{
    BITCOIN_ASSERT(!symbol.empty());

    auto &&key_str = token_cert::get_key(symbol, cert_type);
    const auto key = get_hash(key_str);
    return database_.certs.get(key) != nullptr;
}

bool block_chain_impl::is_candidate_exist(const std::string &symbol)
{
    return get_registered_candidate(symbol) != nullptr;
}

std::shared_ptr<candidate_info> block_chain_impl::get_registered_candidate(const std::string &symbol)
{
    BITCOIN_ASSERT(!symbol.empty());
    // return the registered identifiable token, its status must be CANDIDATE_STATUS_REGISTER
    return database_.candidates.get(get_hash(symbol));
}

std::shared_ptr<candidate_info::list> block_chain_impl::get_registered_candidates()
{
    // return the registered identifiable tokens, their status must be CANDIDATE_STATUS_REGISTER
    return database_.candidates.get_blockchain_candidates();
}

bool block_chain_impl::exist_in_candidates(std::string uid)
{
    auto sh_vec = get_registered_candidates();
    if (nullptr != sh_vec)
    {
        auto pred = [uid](libbitcoin::chain::candidate_info &info) {
            return info.to_uid == uid;
        };
        return std::find_if(sh_vec->begin(), sh_vec->end(), pred) != sh_vec->end();
    }
    return false;
}

std::shared_ptr<candidate_info::list> block_chain_impl::get_candidate_history(
    const std::string &symbol, uint64_t limit, uint64_t page_number)
{
    BITCOIN_ASSERT(!symbol.empty());
    // return the identifiable tokens of specified symbol, the last item's status must be CANDIDATE_STATUS_REGISTER
    return database_.candidate_history.get_history_candidates_by_height(get_short_hash(symbol), 0, 0, limit, page_number);
}

std::shared_ptr<candidate::list> block_chain_impl::get_wallet_candidates(
    const std::string &wallet, const std::string &symbol)
{
    auto sp_vec = std::make_shared<candidate::list>();

    auto pvaddr = get_wallet_addresses(wallet);
    if (!pvaddr)
        return sp_vec;

    chain::transaction tx_temp;
    uint64_t tx_height;

    for (auto &each : *pvaddr)
    {
        bc::wallet::payment_address payment_address(each.get_address());
        auto &&rows = get_address_history(payment_address);

        for (auto &row : rows)
        {
            // spend unconfirmed (or no spend attempted)
            if ((row.spend.hash == null_hash) && get_transaction(row.output.hash, tx_temp, tx_height))
            {
                BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
                const auto &output = tx_temp.outputs.at(row.output.index);
                if (output.is_candidate())
                {
                    auto &&token = output.get_candidate();
                    if (symbol.empty())
                    {
                        sp_vec->emplace_back(std::move(token));
                    }
                    else if (symbol == token.get_symbol())
                    {
                        sp_vec->emplace_back(std::move(token));
                        return sp_vec;
                    }
                }
            }
        }
    }

    return sp_vec;
}

uint64_t block_chain_impl::get_address_token_volume(const std::string &addr, const std::string &token)
{
    uint64_t token_volume = 0;
    auto address = payment_address(addr);
    auto &&rows = get_address_history(address);

    chain::transaction tx_temp;
    uint64_t tx_height;

    for (auto &row : rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash) && get_transaction(tx_temp, tx_height, row.output.hash))
        {
            auto output = tx_temp.outputs.at(row.output.index);
            if ((output.is_token_transfer() || output.is_token_issue() || output.is_token_secondaryissue()))
            {
                if (output.get_token_symbol() == token)
                {
                    token_volume += output.get_token_amount();
                }
            }
        }
    }

    return token_volume;
}

uint64_t block_chain_impl::get_wallet_token_volume(const std::string &wallet, const std::string &token)
{
    uint64_t volume = 0;
    auto pvaddr = get_wallet_addresses(wallet);
    if (pvaddr)
    {
        for (auto &each : *pvaddr)
        {
            volume += get_address_token_volume(each.get_address(), token);
        }
    }

    return volume;
}

uint64_t block_chain_impl::get_token_volume(const std::string &token)
{
    return database_.tokens.get_token_volume(token);
}

std::string block_chain_impl::get_token_symbol_from_asset_data(const asset_data &data)
{
    std::string token_symbol("");
    if (data.get_kind_value() == business_kind::token_issue)
    {
        auto detail = boost::get<token_detail>(data.get_data());
        token_symbol = detail.get_symbol();
    }
    else if (data.get_kind_value() == business_kind::token_transfer)
    {
        auto transfer = boost::get<token_transfer>(data.get_data());
        token_symbol = transfer.get_symbol();
    }
    else if (data.get_kind_value() == business_kind::token_cert)
    {
        auto cert = boost::get<token_cert>(data.get_data());
        token_symbol = cert.get_symbol();
    }
    return token_symbol;
}

// get special tokens of the wallet/name, just used for token_detail/token_transfer
std::shared_ptr<business_history::list> block_chain_impl::get_address_business_history(const std::string &addr,
                                                                                       const std::string &symbol, business_kind kind, uint8_t confirmed)
{
    auto ret_vector = std::make_shared<business_history::list>();
    auto sh_vec = database_.address_tokens.get_address_business_history(addr, 0);

    for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter)
    {
        if ((iter->data.get_kind_value() != kind) || (iter->status != confirmed))
            continue;

        // ucn business process
        if (kind == business_kind::ucn)
        {
            ret_vector->emplace_back(std::move(*iter));
            continue;
        }

        // ucn award business process
        if (kind == business_kind::ucn_award)
        {
            ret_vector->emplace_back(std::move(*iter));
            continue;
        }

        // token business process
        std::string &&token_symbol = get_token_symbol_from_asset_data(iter->data);
        if (symbol == token_symbol)
        {
            ret_vector->emplace_back(std::move(*iter));
        }
    }

    return ret_vector;
}

// get special tokens of the wallet/name, just used for token_detail/token_transfer
std::shared_ptr<business_record::list> block_chain_impl::get_address_business_record(const std::string &addr,
                                                                                     uint64_t start, uint64_t end, const std::string &symbol)
{
    auto ret_vector = std::make_shared<business_record::list>();
    auto sh_vec = database_.address_tokens.get(addr, start, end);

    if (symbol.empty())
    { // all utxo
        for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter)
        {
            ret_vector->emplace_back(std::move(*iter));
        }
    }
    else
    { // token symbol utxo
        for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter)
        {
            // token business process
            std::string &&token_symbol = get_token_symbol_from_asset_data(iter->data);
            if (symbol == token_symbol)
            {
                ret_vector->emplace_back(std::move(*iter));
            }
        }
    }

    return ret_vector;
}

// get special tokens of the wallet/name, just used for token_detail/token_transfer
std::shared_ptr<business_record::list> block_chain_impl::get_address_business_record(const std::string &address,
                                                                                     const std::string &symbol, size_t start_height, size_t end_height, uint64_t limit, uint64_t page_number) const
{
    return database_.address_tokens.get(address, symbol, start_height, end_height, limit, page_number);
}

// get special tokens of the wallet/name, just used for token_detail/token_transfer
std::shared_ptr<business_history::list> block_chain_impl::get_address_business_history(const std::string &addr,
                                                                                       business_kind kind, uint8_t confirmed)
{
    auto sp_token_vec = std::make_shared<business_history::list>();

    business_history::list token_vec = database_.address_tokens.get_business_history(addr, 0, kind, confirmed);
    const auto add_token = [&](const business_history &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };
    std::for_each(token_vec.begin(), token_vec.end(), add_token);

    return sp_token_vec;
}

// get wallet owned business history between begin and end
std::shared_ptr<business_history::list> block_chain_impl::get_wallet_business_history(const std::string &name,
                                                                                      business_kind kind, uint32_t time_begin, uint32_t time_end)
{
    auto wallet_addr_vec = get_wallet_addresses(name);
    auto sp_token_vec = std::make_shared<business_history::list>();

    // copy each token_vec element to sp_token
    const auto add_token = [&](const business_history &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };

    // search all tokens belongs to this address which is owned by wallet
    const auto action = [&](const wallet_address &elem) {
        auto token_vec = database_.address_tokens.get_business_history(elem.get_address(), 0, kind, time_begin, time_end);
        std::for_each(token_vec.begin(), token_vec.end(), add_token);
    };
    std::for_each(wallet_addr_vec->begin(), wallet_addr_vec->end(), action);

    return sp_token_vec;
}

std::shared_ptr<business_history::list> block_chain_impl::get_address_business_history(const std::string &addr,
                                                                                       business_kind kind, uint32_t time_begin, uint32_t time_end)
{
    auto sp_token_vec = std::make_shared<business_history::list>();

    business_history::list token_vec = database_.address_tokens.get_business_history(addr, 0, kind, time_begin, time_end);
    const auto add_token = [&](const business_history &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };
    std::for_each(token_vec.begin(), token_vec.end(), add_token);

    return sp_token_vec;
}

std::shared_ptr<business_history::list> block_chain_impl::get_address_business_history(const std::string &addr)
{
    auto sp_token_vec = std::make_shared<business_history::list>();
    auto key = get_short_hash(addr);
    business_history::list token_vec = database_.address_tokens.get_business_history(key, 0);
    const auto add_token = [&](const business_history &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };
    std::for_each(token_vec.begin(), token_vec.end(), add_token);

    return sp_token_vec;
}

/// get business record according address
std::shared_ptr<business_record::list> block_chain_impl::get_address_business_record(const std::string &addr,
                                                                                     size_t from_height, size_t limit)
{
    auto sp_token_vec = std::make_shared<business_record::list>();
    auto key = get_short_hash(addr);
    business_record::list token_vec = database_.address_tokens.get(key, from_height, limit);
    const auto add_token = [&](const business_record &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };
    std::for_each(token_vec.begin(), token_vec.end(), add_token);

    return sp_token_vec;
}

// get all tokens belongs to the wallet/name
std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_tokens(
    const std::string &name)
{
    return get_wallet_tokens(name, business_kind::unknown);
}

// get special tokens of the wallet/name, just used for token_detail/token_transfer
std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_tokens(
    const std::string &name, business_kind kind)
{
    auto wallet_addr_vec = get_wallet_addresses(name);
    auto sp_token_vec = std::make_shared<business_address_token::list>();

    // copy each token_vec element to sp_token
    const auto add_token = [&](const business_address_token &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };

    // search all tokens belongs to this address which is owned by wallet
    const auto action = [&](const wallet_address &elem) {
        business_address_token::list token_vec = database_.address_tokens.get_tokens(elem.get_address(), 0, kind);
        std::for_each(token_vec.begin(), token_vec.end(), add_token);
    };
    std::for_each(wallet_addr_vec->begin(), wallet_addr_vec->end(), action);

    return sp_token_vec;
}

// get all unissued tokens which stored in local database
std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_tokens()
{
    auto sh_acc_vec = get_wallets();
    auto ret_vector = std::make_shared<business_address_token::list>();

    for (auto &acc : *sh_acc_vec)
    {
        auto sh_vec = get_wallet_tokens(acc.get_name());
        const auto action = [&](const business_address_token &addr_token) {
            ret_vector->emplace_back(std::move(addr_token));
        };
        std::for_each(sh_vec->begin(), sh_vec->end(), action);
    }

    return ret_vector;
}

std::shared_ptr<token_detail> block_chain_impl::get_wallet_unissued_token(const std::string &name,
                                                                          const std::string &symbol)
{
    std::shared_ptr<token_detail> sp_token(nullptr);

    // get wallet token which is not issued (not in blockchain)
    auto no_issued_tokens = database_.wallet_tokens.get_unissued_tokens(get_short_hash(name));
    const auto match = [&](const business_address_token &addr_token) {
        return addr_token.detail.get_symbol() == symbol;
    };
    auto iter = std::find_if(no_issued_tokens->begin(), no_issued_tokens->end(), match);
    if (iter != no_issued_tokens->end())
    {
        sp_token = std::make_shared<token_detail>((*iter).detail);
    }

    return sp_token;
}

// get all local unissued tokens belongs to the wallet/name
std::shared_ptr<business_address_token::list> block_chain_impl::get_wallet_unissued_tokens(const std::string &name)
{
    auto sp_token_vec = std::make_shared<business_address_token::list>();
    auto sp_issues_token_vec = get_issued_tokens();
    // copy each token_vec element which is unissued to sp_token
    const auto add_token = [&sp_token_vec, &sp_issues_token_vec](const business_address_token &addr_token) {
        auto &symbol = addr_token.detail.get_symbol();
        auto pos = std::find_if(sp_issues_token_vec->begin(), sp_issues_token_vec->end(),
                                [&symbol](const token_detail &elem) {
                                    return symbol == elem.get_symbol();
                                });
        if (pos == sp_issues_token_vec->end())
        { // token is unissued in blockchain
            sp_token_vec->emplace_back(std::move(addr_token));
        }
    };

    // get wallet token which is not issued (not in blockchain)
    auto no_issued_tokens = database_.wallet_tokens.get_unissued_tokens(get_short_hash(name));
    std::for_each(no_issued_tokens->begin(), no_issued_tokens->end(), add_token);

    return sp_token_vec;
}

/// get all the token in blockchain
std::shared_ptr<token_detail::list> block_chain_impl::get_issued_tokens(const std::string &symbol)
{
    auto sp_vec = std::make_shared<token_detail::list>();
    const auto is_symbol_empty = symbol.empty();
    if (!is_symbol_empty && bc::wallet::symbol::is_forbidden(symbol))
    {
        return sp_vec;
    }
    auto sp_blockchain_vec = database_.tokens.get_blockchain_tokens(symbol);
    for (auto &each : *sp_blockchain_vec)
    {
        auto &token = each.get_token();
        if (is_symbol_empty && bc::wallet::symbol::is_forbidden(token.get_symbol()))
        {
            // swallow forbidden symbol
            continue;
        }

        sp_vec->push_back(token);
    }
    return sp_vec;
}

std::shared_ptr<blockchain_token::list> block_chain_impl::get_token_register_output(const std::string &symbol)
{
    return database_.tokens.get_token_history(symbol);
}

/* check uid symbol exist or not
*/
bool block_chain_impl::is_uid_exist(const std::string &uid_name)
{
    // find from blockchain database
    return get_registered_uid(uid_name) != nullptr;
}

/* check uid address exist or not
*/
bool block_chain_impl::is_address_registered_uid(const std::string &uid_address, uint64_t fork_index)
{
    return !get_uid_from_address(uid_address, fork_index).empty();
}

bool block_chain_impl::is_wallet_owned_uid(const std::string &wallet, const std::string &symbol)
{
    auto uid_detail = get_registered_uid(symbol);
    if (!uid_detail)
    {
        return false;
    }

    auto pvaddr = get_wallet_addresses(wallet);
    if (pvaddr)
    {
        const auto match = [&uid_detail](const wallet_address &item) {
            return item.get_address() == uid_detail->get_address();
        };
        auto iter = std::find_if(pvaddr->begin(), pvaddr->end(), match);
        return iter != pvaddr->end();
    }

    return false;
}

std::shared_ptr<uid_detail::list> block_chain_impl::get_wallet_uids(const std::string &wallet)
{
    auto sh_vec = std::make_shared<uid_detail::list>();
    auto pvaddr = get_wallet_addresses(wallet);
    if (pvaddr)
    {
        for (const auto &wallet_address : *pvaddr)
        {
            auto uid_address = wallet_address.get_address();
            auto uid_symbol = get_uid_from_address(uid_address);
            if (!uid_symbol.empty())
            {
                sh_vec->emplace_back(uid_detail(uid_symbol, uid_address));
            }
        }
    }

    return sh_vec;
}
/* find uid by address
*/
std::string block_chain_impl::get_uid_from_address(const std::string &uid_address, uint64_t fork_index)
{
    //search from address_uid_database first
    business_address_uid::list uid_vec = database_.address_uids.get_uids(uid_address, 0, fork_index);

    std::string uid_symbol = "";
    if (!uid_vec.empty())
        uid_symbol = uid_vec[0].detail.get_symbol();

    if (uid_symbol != "")
    {
        //double check
        std::shared_ptr<blockchain_uid::list> blockchain_uidlist = get_uid_history_addresses(uid_symbol);
        auto iter = std::find_if(blockchain_uidlist->begin(), blockchain_uidlist->end(),
                                 [fork_index](const blockchain_uid &item) {
                                     return is_blackhole_address(item.get_uid().get_address()) || item.get_height() <= fork_index;
                                 });

        if (iter == blockchain_uidlist->end() || iter->get_uid().get_address() != uid_address)
            return "";
    }
    else if (uid_symbol == "" && fork_index != max_uint64)
    {
        // search from uids database
        auto sp_uid_vec = database_.uids.getuids_from_address_history(uid_address, 0, fork_index);
        if (sp_uid_vec && !sp_uid_vec->empty())
        {
            uid_symbol = sp_uid_vec->back().get_uid().get_symbol();
        }
    }

    return uid_symbol;
}

/* find history addresses by the uid symbol
*/
std::shared_ptr<blockchain_uid::list> block_chain_impl::get_uid_history_addresses(const std::string &symbol)
{
    const auto hash = get_hash(symbol);
    return database_.uids.get_history_uids(hash);
}

std::shared_ptr<uid_detail> block_chain_impl::get_registered_uid(const std::string &symbol)
{
    std::shared_ptr<uid_detail> sp_uid(nullptr);
    const auto hash = get_hash(symbol);
    auto sh_block_uid = database_.uids.get(hash);
    if (sh_block_uid)
    {
        sp_uid = std::make_shared<uid_detail>(sh_block_uid->get_uid());
    }
    return sp_uid;
}

/// get all the uid in blockchain
std::shared_ptr<uid_detail::list> block_chain_impl::get_registered_uids()
{
    auto sp_vec = std::make_shared<uid_detail::list>();
    if (!sp_vec)
        return nullptr;

    auto sp_blockchain_vec = database_.uids.get_blockchain_uids();
    for (const auto &each : *sp_blockchain_vec)
    {
        if (each.get_status() == blockchain_uid::address_current)
        {
            sp_vec->emplace_back(each.get_uid());
        }
    }

    return sp_vec;
}

std::shared_ptr<token_detail> block_chain_impl::get_issued_token(const std::string &symbol)
{
    std::shared_ptr<token_detail> sp_token(nullptr);
    const auto hash = get_hash(symbol);
    auto sh_block_token = database_.tokens.get(hash);
    if (sh_block_token)
    {
        sp_token = std::make_shared<token_detail>(sh_block_token->get_token());
    }
    return sp_token;
}

// get all addresses
std::shared_ptr<wallet_address::list> block_chain_impl::get_addresses()
{
    auto sh_acc_vec = get_wallets();
    auto ret_vector = std::make_shared<wallet_address::list>();
    const auto action = [&](const wallet_address &addr) {
        ret_vector->emplace_back(std::move(addr));
    };

    for (auto &acc : *sh_acc_vec)
    {
        auto sh_vec = get_wallet_addresses(acc.get_name());
        std::for_each(sh_vec->begin(), sh_vec->end(), action);
    }

    return ret_vector;
}

std::shared_ptr<business_address_message::list> block_chain_impl::get_wallet_messages(const std::string &name)
{
    auto wallet_addr_vec = get_wallet_addresses(name);
    auto sp_token_vec = std::make_shared<business_address_message::list>();

    // copy each token_vec element to sp_token
    const auto add_token = [&](const business_address_message &addr_token) {
        sp_token_vec->emplace_back(std::move(addr_token));
    };

    // search all tokens belongs to this address which is owned by wallet
    const auto action = [&](const wallet_address &elem) {
        business_address_message::list token_vec = database_.address_tokens.get_messages(elem.get_address(), 0);
        std::for_each(token_vec.begin(), token_vec.end(), add_token);
    };
    std::for_each(wallet_addr_vec->begin(), wallet_addr_vec->end(), action);

    return sp_token_vec;
}

void block_chain_impl::fired()
{
    organizer_.fired();
}

/* find token symbol exist or not
*  find steps:
*  1. find from blockchain
*  2. find from local database(includes wallet created token) if check_local_db = true
*/
bool block_chain_impl::is_token_exist(const std::string &token_name, bool check_local_db)
{
    // 1. find from blockchain database
    if (get_issued_token(token_name))
        return true;

    // 2. find from local database token
    if (check_local_db)
    {
        auto sh_acc_vec = get_local_tokens();
        // scan all wallet token
        for (auto &acc : *sh_acc_vec)
        {
            if (token_name == acc.get_symbol())
                return true;
        }
    }

    return false;
}

uint64_t block_chain_impl::get_token_height(const std::string &token_name) const
{
    return database_.tokens.get_register_height(token_name);
}

uint64_t block_chain_impl::get_uid_height(const std::string &uid_name) const
{
    return database_.uids.get_register_height(uid_name);
}

uint64_t block_chain_impl::get_token_cert_height(const std::string &cert_symbol, const token_cert_type &cert_type)
{
    auto &&key_str = token_cert::get_key(cert_symbol, cert_type);
    const auto key = get_hash(key_str);
    auto cert = database_.certs.get(key);
    if (cert)
    {
        business_history::list history_cert = database_.address_tokens.get_token_certs_history(cert->get_address(), cert_symbol, cert_type, 0);
        if (history_cert.size() > 0)
        {
            return history_cert.back().output_height;
        }
    }
    return max_uint64;
}

uint64_t block_chain_impl::get_candidate_height(const std::string &candidate_symbol) const
{
    return database_.candidates.get_register_height(candidate_symbol);
}

/// get token from local database including all wallet's tokens
std::shared_ptr<token_detail::list> block_chain_impl::get_local_tokens()
{
    auto ret_vec = std::make_shared<token_detail::list>();
    auto sh_acc_vec = get_wallets();
    // scan all wallet token -- maybe the token has been issued
    for (auto &acc : *sh_acc_vec)
    {
        auto no_issued_tokens = database_.wallet_tokens.get(get_short_hash(acc.get_name()));
        for (auto &detail : no_issued_tokens)
        {
            ret_vec->emplace_back(std::move(detail));
        }
    }

    return ret_vec;
}

void block_chain_impl::uppercase_symbol(std::string &symbol)
{
    // uppercase symbol
    for (auto &i : symbol)
    {
        if (!(std::isalnum(i) || i == '.'))
            throw std::logic_error{"symbol must be alpha or number or dot"};
        i = std::toupper(i);
    }
}

bool block_chain_impl::is_valid_address(const std::string &address)
{
    return is_payment_address(address) ||
           is_script_address(address) ||
           is_stealth_address(address) ||
           is_blackhole_address(address);
}

bool block_chain_impl::is_blackhole_address(const std::string &address)
{
    return (address == bc::wallet::payment_address::blackhole_address);
}

bool block_chain_impl::is_stealth_address(const std::string &address)
{
    bc::wallet::stealth_address addr{address};
    return (addr && (addr.version() == bc::wallet::stealth_address::mainnet_p2kh));
}

bool block_chain_impl::is_payment_address(const std::string &address)
{
    bc::wallet::payment_address addr{address};
    return (addr && (addr.version() == bc::wallet::payment_address::mainnet_p2kh));
}

// stupid name for this function. should be is_p2sh_address.
bool block_chain_impl::is_script_address(const std::string &address)
{
    bc::wallet::payment_address addr{address};
    return (addr && (addr.version() == bc::wallet::payment_address::mainnet_p2sh));
}

organizer &block_chain_impl::get_organizer()
{
    return organizer_;
}

bool block_chain_impl::get_transaction(const hash_digest &hash,
                                       chain::transaction &tx, uint64_t &tx_height)
{

    bool ret = false;
    if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

    const auto result = database_.transactions.get(hash);
    if (result)
    {
        tx = result.transaction();
        tx_height = result.height();
        ret = true;
    }
    else
    {
        boost::mutex mutex;
        tx_message::ptr tx_ptr = nullptr;

        mutex.lock();
        auto f = [&tx_ptr, &mutex](const code &ec, tx_message::ptr tx_) -> void {
            if ((code)error::success == ec)
                tx_ptr = tx_;
            mutex.unlock();
        };

        pool().fetch(hash, f);
        boost::unique_lock<boost::mutex> lock(mutex);
        if (tx_ptr)
        {
            tx = *(static_cast<std::shared_ptr<chain::transaction>>(tx_ptr));
            tx_height = 0;
            ret = true;
        }
    }
#ifdef UC_DEBUG
    log::debug("get_transaction=") << tx.to_string(1);
#endif

    return ret;
}

bool block_chain_impl::get_transaction_callback(const hash_digest &hash,
                                                std::function<void(const code &, const chain::transaction &)> handler)
{

    bool ret = false;
    if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

    const auto result = database_.transactions.get(hash);
    if (result)
    {
        handler(error::success, result.transaction());
        ret = true;
    }
    else
    {
        tx_message::ptr tx_ptr = nullptr;

        auto f = [&tx_ptr, handler](const code &ec, tx_message::ptr tx_) -> void {
            if ((code)error::success == ec)
            {
                tx_ptr = tx_;
                if (tx_ptr)
                    handler(ec, *(static_cast<std::shared_ptr<chain::transaction>>(tx_ptr)));
            }
        };

        pool().fetch(hash, f);
        if (tx_ptr)
        {
            ret = true;
        }
    }

    return ret;
}

bool block_chain_impl::get_history_callback(const payment_address &address,
                                            size_t limit, size_t from_height,
                                            std::function<void(const code &, chain::history::list &)> handler)
{

    bool ret = false;
    if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

    auto f = [&ret, handler](const code &ec, chain::history_compact::list compact) -> void {
        if ((code)error::success == ec)
        {
            history::list result;

            // Process and remove all outputs.
            for (auto output = compact.begin(); output != compact.end();)
            {
                if (output->kind == point_kind::output)
                {
                    history row;
                    row.output = output->point;
                    row.output_height = output->height;
                    row.value = output->value;
                    row.spend = {null_hash, max_uint32};
                    row.temporary_checksum = output->point.checksum();
                    result.emplace_back(row);
                    output = compact.erase(output);
                    continue;
                }

                ++output;
            }

            // All outputs have been removed, process the spends.
            for (const auto &spend : compact)
            {
                auto found = false;

                // Update outputs with the corresponding spends.
                for (auto &row : result)
                {
                    if (row.temporary_checksum == spend.previous_checksum &&
                        row.spend.hash == null_hash)
                    {
                        row.spend = spend.point;
                        row.spend_height = spend.height;
                        found = true;
                        break;
                    }
                }

                // This will only happen if the history height cutoff comes between
                // an output and its spend. In this case we return just the spend.
                if (!found)
                {
                    history row;
                    row.output = {null_hash, max_uint32};
                    row.output_height = max_uint64;
                    row.value = max_uint64;
                    row.spend = spend.point;
                    row.spend_height = spend.height;
                    result.emplace_back(row);
                }
            }

            compact.clear();

            // Clear all remaining checksums from unspent rows.
            for (auto &row : result)
                if (row.spend.hash == null_hash)
                    row.spend_height = max_uint64;

            // TODO: sort by height and index of output, spend or both in order.
            handler(ec, result);
            ret = true;
        }
    };

    // Obtain payment address history from the transaction pool and blockchain.
    pool().fetch_history(address, limit, from_height, f);

    return ret;
}

code block_chain_impl::validate_tx_engine(const chain::transaction &tx)
{
    code ret = error::success;
    if (stopped())
    {
        //handler(error::service_stopped, {});
        log::debug("validate_tx_engine") << "ec=error::service_stopped";
        ret = error::service_stopped;
        return ret;
    }

    //std::shared_ptr<tx_message>
    auto tx_ptr = std::make_shared<tx_message>(tx);
    boost::mutex mutex;

    mutex.lock();
    auto f = [&ret, &mutex](const code &ec, tx_message::ptr tx_, chain::point::indexes idx_vec) -> void {
        log::debug("validate_tx_engine") << "ec=" << ec << " idx_vec=" << idx_vec.size();
        log::debug("validate_tx_engine") << "ec.message=" << ec.message();
        //if((error::success == ec) && idx_vec.empty())
        ret = ec;
        mutex.unlock();
    };

    pool().validate(tx_ptr, f);
    boost::unique_lock<boost::mutex> lock(mutex);

    return ret;
}

code block_chain_impl::broadcast_transaction(const chain::transaction &tx)
{
    code ret = error::success;
    if (stopped())
    {
        //handler(error::service_stopped, {});
        log::debug("broadcast_transaction") << "ec=error::service_stopped";
        ret = error::service_stopped;
        return ret;
    }

    //std::shared_ptr<tx_message>
    using transaction_ptr = std::shared_ptr<tx_message>;
    auto tx_ptr = std::make_shared<tx_message>(tx);
    boost::mutex valid_mutex;

    valid_mutex.lock();
    //send_mutex.lock();

    pool().store(tx_ptr, [tx_ptr](const code &ec, transaction_ptr) {
        //send_mutex.unlock();
        //ret = true;
        log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " confirmed"; }, [&valid_mutex, &ret, tx_ptr](const code &ec, std::shared_ptr<tx_message>, chain::point::indexes idx_vec) {
        log::debug("broadcast_transaction") << "ec=" << ec << " idx_vec=" << idx_vec.size();
        log::debug("broadcast_transaction") << "ec.message=" << ec.message();
        ret = ec;
        if(error::success == ec){
            log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " validated";
        } else {
            //send_mutex.unlock(); // incase dead lock
            log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " invalidated";
        }
        valid_mutex.unlock(); });
    boost::unique_lock<boost::mutex> lock(valid_mutex);
    //boost::unique_lock<boost::mutex> send_lock(send_mutex);

    return ret;
}

bool block_chain_impl::get_history(const bc::wallet::payment_address &address,
                                   uint64_t limit, uint64_t from_height, history_compact::list &history)
{
    if (stopped())
    {
        //handler(error::service_stopped, {});
        return false;
    }

    boost::mutex mutex;

    mutex.lock();
    auto f = [&history, &mutex](const code &ec, const history_compact::list &history_) -> void {
        if ((code)error::success == ec)
            history = history_;
        mutex.unlock();
    };

    // Obtain payment address history from the transaction pool and blockchain.
    pool().fetch_history(address, limit, from_height, f);
    boost::unique_lock<boost::mutex> lock(mutex);

#ifdef UC_DEBUG
    log::debug("get_history=") << history.size();
#endif

    return true;
}

bool block_chain_impl::get_tx_inputs_ucn_value(chain::transaction &tx, uint64_t &ucn_val)
{
    chain::transaction tx_temp;
    uint64_t tx_height;
    ucn_val = 0;

    for (auto &each : tx.inputs)
    {

        if (get_transaction(each.previous_output.hash, tx_temp, tx_height))
        {
            auto output = tx_temp.outputs.at(each.previous_output.index);
            ucn_val += output.value;
        }
        else
        {
            log::debug("get_tx_inputs_ucn_value=") << each.to_string(true);
            return false;
        }
    }

    return true;
}

void block_chain_impl::safe_store_wallet(libbitcoin::chain::wallet &acc, const std::vector<std::shared_ptr<wallet_address>> &addresses)
{
    if (stopped())
        return;

    for (auto &address : addresses)
    {
        const auto hash = get_short_hash(address->get_name());
        database_.wallet_addresses.safe_store(hash, *address);
    }
    database_.wallet_addresses.sync();

    database_.wallets.store(acc);
    database_.wallets.sync();
}

} // namespace blockchain
} // namespace libbitcoin
