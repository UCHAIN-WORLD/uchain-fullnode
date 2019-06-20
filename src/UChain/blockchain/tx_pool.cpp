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
#include <UChain/blockchain/tx_pool.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <system_error>
#include <UChain/coin.hpp>
#include <UChain/blockchain/block_chain.hpp>
#include <UChain/blockchain/settings.hpp>
#include <UChain/blockchain/validate_tx_engine.hpp>

namespace libbitcoin
{
namespace blockchain
{

#define NAME "mempool"

using namespace chain;
using namespace wallet;
using namespace std::placeholders;

tx_pool::tx_pool(threadpool &pool, block_chain &chain,
                                   const settings &settings)
    : stopped_(true),
      maintain_consistency_(settings.tx_pool_consistency),
      buffer_(settings.tx_pool_capacity),
      dispatch_(pool, NAME),
      blockchain_(chain),
      index_(pool, chain),
      subscriber_(std::make_shared<transaction_subscriber>(pool, NAME))
{
}

tx_pool::~tx_pool()
{
    clear(error::service_stopped);
}

void tx_pool::start()
{
    stopped_ = false;
    index_.start();
    subscriber_->start();

    // Subscribe to blockchain (organizer) reorg notifications.
    blockchain_.subscribe_reorganize(
        std::bind(&tx_pool::handle_reorganized,
                  this, _1, _2, _3, _4));
}

// The subscriber is not restartable.
void tx_pool::stop()
{
    stopped_ = true;
    index_.stop();
    subscriber_->stop();
    subscriber_->invoke(error::service_stopped, {}, {});
}

void tx_pool::fired()
{
    subscriber_->relay(error::mock, {}, {});
}

bool tx_pool::stopped()
{
    return stopped_;
}

void tx_pool::inventory(message::inventory::ptr inventory)
{
    ///////////////////////////////////////////////////////////////////////////
    // TODO: populate the inventory vector from the full memory pool.
    ///////////////////////////////////////////////////////////////////////////
}

void tx_pool::validate(transaction_ptr tx, validate_handler handler)
{
    dispatch_.ordered(&tx_pool::do_validate,
                      this, tx, handler);
}

void tx_pool::do_validate(transaction_ptr tx,
                                   validate_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, tx, {});
        return;
    }

    const auto validate = std::make_shared<validate_tx_engine>(
        blockchain_, *tx, *this, dispatch_);

    validate->start(
        dispatch_.ordered_delegate(&tx_pool::handle_validated,
                                   this, _1, _2, _3, handler));
}

void tx_pool::handle_validated(const code &ec, transaction_ptr tx,
                                        const indexes &unconfirmed, validate_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, tx, {});
        return;
    }

    if (ec == (code)error::input_not_found || ec == (code)error::validate_inputs_failed)
    {
        BITCOIN_ASSERT(unconfirmed.size() == 1);
        handler(ec, tx, unconfirmed);
        return;
    }

    if (ec)
    {
        BITCOIN_ASSERT(unconfirmed.empty());
        handler(ec, tx, {});
        return;
    }

    // Recheck the memory pool, as a duplicate may have been added.
    if (is_in_pool(tx->hash()))
    {
        handler(error::duplicate, tx, {});
        return;
    }

    code error = check_symbol_repeat(tx);
    if (error != error::success)
    {
        handler(error, tx, {});
        return;
    }

    handler(error::success, tx, unconfirmed);
}

code tx_pool::check_symbol_repeat(transaction_ptr tx)
{
    std::set<string> tokens;
    std::set<string> token_certs;
    std::set<string> candidates;
    std::set<string> uids;
    std::set<string> uidaddreses;
    std::set<string> uidattaches;

    auto check_outputs = [&](transaction_ptr txs) -> code {
        for (auto &output : txs->outputs)
        {
            //add asset check;avoid send with uid while transfer
            if (output.attach_data.get_version() == UID_ASSET_VERIFY_VERSION)
            {
                auto check_uid = [&uids, &uidattaches](string attach_uid) {
                    if (!attach_uid.empty() && uids.find(attach_uid) != uids.end())
                    {
                        log::debug(LOG_BLOCKCHAIN)
                            << "check_symbol_repeat asset uid: " + attach_uid
                            << " already exists in txpool!";
                        return false;
                    }

                    uidattaches.insert(attach_uid);
                    return true;
                };

                if (!check_uid(output.attach_data.get_from_uid()) || !check_uid(output.attach_data.get_to_uid()))
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat from_uid " + output.attach_data.get_from_uid()
                        << " to_uid " + output.attach_data.get_to_uid()
                        << " check failed!"
                        << " " << tx->to_string(1);
                    return error::uid_exist;
                }
            }

            if (output.is_token_issue())
            {
                auto r = tokens.insert(output.get_token_symbol());
                if (r.second == false)
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat token " + output.get_token_symbol()
                        << " already exists in txpool!"
                        << " " << tx->to_string(1);
                    return error::token_exist;
                }
            }
            else if (output.is_token_cert())
            {
                auto &&key = output.get_token_cert().get_key();
                auto r = token_certs.insert(key);
                if (r.second == false)
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat cert " + output.get_token_cert_symbol()
                        << " with type " << output.get_token_cert_type()
                        << " already exists in txpool!"
                        << " " << tx->to_string(1);
                    return error::token_cert_exist;
                }
            }
            else if (output.is_candidate())
            {
                auto r = candidates.insert(output.get_token_symbol());
                if (r.second == false)
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat candidate " + output.get_token_symbol()
                        << " already exists in txpool!"
                        << " " << tx->to_string(1);
                    return error::candidate_exist;
                }
            }
            else if (output.is_uid())
            {
                auto uidsymbol = output.get_uid_symbol();
                auto uidexist = uids.insert(uidsymbol);
                if (uidexist.second == false)
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat uid " + uidsymbol
                        << " already exists in txpool!"
                        << " " << tx->to_string(1);
                    return error::uid_exist;
                }

                auto uidaddress = uidaddreses.insert(output.get_uid_address());
                if (uidaddress.second == false)
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat uid address " + output.get_uid_address()
                        << " already has uid on it in txpool!"
                        << " " << tx->to_string(1);
                    return error::address_registered_uid;
                }

                if (uidattaches.find(uidsymbol) != uidattaches.end())
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_symbol_repeat asset uid: " + uidsymbol
                        << " already transfer in txpool!"
                        << " " << tx->to_string(1);
                    return error::uid_exist;
                }
            }
        }
        return error::success;
    };

    code ec;
    for (auto &item : buffer_)
    {
        if (!item.tx)
            continue;

        if ((ec = check_outputs(item.tx)) != error::success)
            break;
    }

    return ec != error::success ? ec : check_outputs(tx);
}

// handle_confirm will never fire if handle_validate returns a failure code.
void tx_pool::store(transaction_ptr tx,
                             confirm_handler handle_confirm, validate_handler handle_validate)
{
    if (stopped())
    {
        handle_validate(error::service_stopped, tx, {});
        return;
    }

    validate(tx,
             std::bind(&tx_pool::do_store,
                       this, _1, _2, _3, handle_confirm, handle_validate));
}

// This is overly complex due to the transaction pool and index split.
void tx_pool::do_store(const code &ec, transaction_ptr tx,
                                const indexes &unconfirmed, confirm_handler handle_confirm,
                                validate_handler handle_validate)
{
    if (ec)
    {
        handle_validate(ec, tx, {});
        return;
    }

    // Set up deindexing to run after transaction pool removal.
    const auto do_deindex = [this, handle_confirm](const code ec,
                                                   transaction_ptr tx) {
        const auto do_confirm = [handle_confirm, tx, ec](const code) {
            handle_confirm(ec, tx);
        };

        // This always sets success but we have captured the confirmation code.
        index_.remove(*tx, do_confirm);
    };

    // Add to pool, save confirmation handler.
    add(tx, do_deindex);

    const auto handle_indexed = [this, handle_validate, tx, unconfirmed](
                                    const code ec) {
        // Notify subscribers that the tx has been validated and indexed.
        notify_transaction(unconfirmed, tx);

        log::debug(LOG_BLOCKCHAIN)
            << "Transaction saved to mempool (" << buffer_.size() << ")";

        // Notify caller that the tx has been validated and indexed.
        handle_validate(ec, tx, unconfirmed);
    };

    // Add to index and invoke handler to indicate validation and indexing.
    index_.add(*tx, handle_indexed);
}

void tx_pool::fetch(fetch_all_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto tx_fetcher = [this, handler]() {
        std::vector<transaction_ptr> transactions;
        for (auto item : buffer_)
        {
            if (item.tx)
                transactions.push_back(item.tx);
        }
        handler(error::success, transactions);
    };

    dispatch_.ordered(tx_fetcher);
}

void tx_pool::delete_tx(const hash_digest &tx_hash)
{
    if (stopped())
    {
        return;
    }

    log::debug(LOG_BLOCKCHAIN) << " delete_tx hash:" << libbitcoin::encode_hash(tx_hash);
    const auto tx_delete = [this, tx_hash]() {
        for (auto item = buffer_.begin(); item != buffer_.end(); ++item)
        {
            if (item->tx->hash() == tx_hash)
            {
                log::debug(LOG_BLOCKCHAIN) << " delete_tx hash:" << libbitcoin::encode_hash(tx_hash) << " success";
                buffer_.erase(item);
                break;
            }
        }
    };

    dispatch_.ordered(tx_delete);
}

void tx_pool::fetch(const hash_digest &transaction_hash,
                             fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto tx_fetcher = [this, transaction_hash, handler]() {
        const auto it = find(transaction_hash);

        if (it == buffer_.end())
            handler(error::not_found, {});
        else
            handler(error::success, it->tx);
    };

    dispatch_.ordered(tx_fetcher);
}

void tx_pool::fetch_history(const payment_address &address,
                                     size_t limit, size_t from_height,
                                     block_chain::history_fetch_handler handler)
{
    // This passes through to blockchain to build combined history.
    index_.fetch_all_history(address, limit, from_height, handler);
}

// TODO: use hash table pool to eliminate this O(n^2) search.
void tx_pool::filter(get_data_ptr message, result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto filter_transactions = [this, message, handler]() {
        auto &inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_transaction_type() && is_in_pool(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        handler(error::success);
    };

    dispatch_.ordered(filter_transactions);
}

void tx_pool::exists(const hash_digest &tx_hash,
                              result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto get_existence = [this, tx_hash, handler]() {
        handler(is_in_pool(tx_hash) ? error::success : error::not_found);
    };

    dispatch_.ordered(get_existence);
}

// new blocks come in - remove txs in new
// old blocks taken out - resubmit txs in old
bool tx_pool::handle_reorganized(const code &ec, size_t fork_point,
                                          const block_list &new_blocks, const block_list &replaced_blocks)
{
    if (ec == (code)error::service_stopped)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "Stopping transaction pool: " << ec.message();
        return false;
    }

    if (ec == error::mock)
        return true;

    if (ec)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "Failure in tx pool reorganize handler: " << ec.message();
        return false;
    }

    log::debug(LOG_BLOCKCHAIN)
        << "Reorganize: tx pool size (" << buffer_.size()
        << ") forked at (" << fork_point
        << ") new blocks (" << new_blocks.size()
        << ") replace blocks (" << replaced_blocks.size() << ")";

    if (replaced_blocks.empty())
    {
        // Remove memory pool transactions that also exist in new blocks.
        dispatch_.ordered(
            std::bind(&tx_pool::remove,
                      this, new_blocks));
    }
    else
    {
        // See http://www.jwz.org/doc/worse-is-better.html
        // for why we take this approach. We return with an error_code.
        // An alternative would be resubmit all tx from the cleared blocks.
        dispatch_.ordered(
            std::bind(&tx_pool::clear,
                      this, error::blockchain_reorganized));
    }

    return true;
}

void tx_pool::subscribe_transaction(
    transaction_handler handle_transaction)
{
    subscriber_->subscribe(handle_transaction, error::service_stopped, {}, {});
}

void tx_pool::notify_transaction(const point::indexes &unconfirmed,
                                          transaction_ptr tx)
{
    subscriber_->relay(error::success, unconfirmed, tx);
}

// Entry methods.
// ----------------------------------------------------------------------------

// A new transaction has been received, add it to the memory pool.
void tx_pool::add(transaction_ptr tx, confirm_handler handler)
{
    // When a new tx is added to the buffer drop the oldest.
    if (maintain_consistency_ && buffer_.size() == buffer_.capacity())
        delete_package(error::pool_filled);

    buffer_.push_back({tx, handler});
}

// There has been a reorg, clear the memory pool using the given reason code.
void tx_pool::clear(const code &ec)
{
    for (const auto &entry : buffer_)
        entry.handle_confirm(ec, entry.tx);

    buffer_.clear();
}

// Delete memory pool txs that are obsoleted by a new block acceptance.
void tx_pool::remove(const block_list &blocks)
{
    // Delete by hash sets a success code.
    delete_confirmed_in_blocks(blocks);

    // Delete by spent sets a double-spend error.
    if (maintain_consistency_)
        delete_spent_in_blocks(blocks);
}

// Consistency methods.
// ----------------------------------------------------------------------------

// Delete mempool txs that are duplicated in the new blocks.
void tx_pool::delete_confirmed_in_blocks(const block_list &blocks)
{
    if (stopped() || buffer_.empty())
        return;

    for (const auto block : blocks)
        for (const auto &tx : block->transactions)
            delete_single(tx.hash(), error::success);
}

// Delete all txs that spend a previous output of any tx in the new blocks.
void tx_pool::delete_spent_in_blocks(const block_list &blocks)
{
    if (stopped() || buffer_.empty())
        return;

    for (const auto block : blocks)
        for (const auto &tx : block->transactions)
            for (const auto &input : tx.inputs)
                delete_dependencies(input.previous_output,
                                    error::double_spend);
}

// Delete any tx that spends any output of this tx.
void tx_pool::delete_dependencies(const output_point &point,
                                           const code &ec)
{
    const auto comparitor = [&point](const input &input) {
        return input.previous_output == point;
    };

    delete_dependencies(comparitor, ec);
}

// Delete any tx that spends any output of this tx.
void tx_pool::delete_dependencies(const hash_digest &tx_hash,
                                           const code &ec)
{
    const auto comparitor = [&tx_hash](const input &input) {
        return input.previous_output.hash == tx_hash;
    };

    delete_dependencies(comparitor, ec);
}

// This is horribly inefficient, but it's simple.
// TODO: Create persistent multi-indexed memory pool (including age and
// children) and perform this pruning trivialy (and add policy over it).
void tx_pool::delete_dependencies(input_compare is_dependency,
                                           const code &ec)
{
    std::vector<entry> dependencies;
    for (const auto &entry : buffer_)
        for (const auto &input : entry.tx->inputs)
            if (is_dependency(input))
            {
                dependencies.push_back(entry);
                break;
            }

    // We queue deletion to protect the iterator.
    for (const auto &dependency : dependencies)
        delete_package(dependency.tx, ec);
}

void tx_pool::delete_package(const code &ec)
{
    if (stopped() || buffer_.empty())
        return;

    // Must copy the entry because it is going to be deleted from the list.
    const auto oldest = buffer_.front();

    oldest.handle_confirm(ec, oldest.tx);
    delete_package(oldest.tx, ec);
}

void tx_pool::delete_package(transaction_ptr tx, const code &ec)
{
    if (delete_single(tx->hash(), ec))
        delete_dependencies(tx->hash(), ec);
}

bool tx_pool::delete_single(const hash_digest &tx_hash, const code &ec)
{
    if (stopped())
        return false;

    const auto matched = [&tx_hash](const entry &entry) {
        return entry.tx->hash() == tx_hash;
    };

    const auto it = std::find_if(buffer_.begin(), buffer_.end(), matched);

    if (it == buffer_.end())
        return false;

    it->handle_confirm(ec, it->tx);
    buffer_.erase(it);

    while (1)
    {
        const auto it = std::find_if(buffer_.begin(), buffer_.end(), matched);

        if (it == buffer_.end())
            break;

        it->handle_confirm(ec, it->tx);
        buffer_.erase(it);
    }

    return true;
}

bool tx_pool::find(transaction_ptr &out_tx,
                            const hash_digest &tx_hash) const
{
    const auto it = find(tx_hash);
    const auto found = it != buffer_.end();

    if (found)
        out_tx = it->tx;

    return found;
}

bool tx_pool::find(chain::transaction &out_tx,
                            const hash_digest &tx_hash) const
{
    const auto it = find(tx_hash);
    const auto found = it != buffer_.end();

    if (found)
    {
        // TRANSACTION COPY
        out_tx = *(it->tx);
    }

    return found;
}

tx_pool::const_iterator tx_pool::find(
    const hash_digest &tx_hash) const
{
    const auto found = [&tx_hash](const entry &entry) {
        return entry.tx->hash() == tx_hash;
    };

    return std::find_if(buffer_.begin(), buffer_.end(), found);
}

bool tx_pool::is_in_pool(const hash_digest &tx_hash) const
{
    return find(tx_hash) != buffer_.end();
}

bool tx_pool::is_spent_in_pool(transaction_ptr tx) const
{
    return is_spent_in_pool(*tx);
}

bool tx_pool::is_spent_in_pool(const transaction &tx) const
{
    const auto found = [this](const input &input) {
        return is_spent_in_pool(input.previous_output);
    };

    const auto &inputs = tx.inputs;
    return std::any_of(inputs.begin(), inputs.end(), found);
}

bool tx_pool::is_spent_in_pool(const output_point &outpoint) const
{
    const auto found = [&outpoint](const entry &entry) {
        return is_spent_by_tx(outpoint, entry.tx);
    };

    return std::any_of(buffer_.begin(), buffer_.end(), found);
}

bool tx_pool::is_spent_by_tx(const output_point &outpoint,
                                      transaction_ptr tx)
{
    const auto found = [&outpoint](const input &input) {
        return input.previous_output == outpoint;
    };

    const auto &inputs = tx->inputs;
    return std::any_of(inputs.begin(), inputs.end(), found);
}

} // namespace blockchain
} // namespace libbitcoin
