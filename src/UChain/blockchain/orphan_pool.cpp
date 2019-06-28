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
#include <UChain/blockchain/orphan_pool.hpp>

#include <algorithm>
#include <cstddef>
#include <UChain/blockchain/block_info.hpp>

namespace libbitcoin
{
namespace blockchain
{

orphan_pool::orphan_pool(size_t capacity)
{
    buffer_.reserve(capacity == 0 ? 1 : capacity);
}

// There is no validation whatsoever of the block up to this pont.
bool orphan_pool::add(block_info::ptr block)
{
    const auto &header = block->actual()->header;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    // No duplicates allowed.
    if (exists(header))
    {
        mutex_.unlock_upgrade();
        //-----------------------------------------------------------------
        return false;
    }

    const auto old_size = buffer_.size();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();
    buffer_.push_back(block);
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    log::debug(LOG_BLOCKCHAIN)
        << "Orphan pool added block [" << encode_hash(block->hash())
        << "] previous [" << encode_hash(header.previous_block_hash)
        << "] old size (" << old_size << ").";

    return true;
}

void orphan_pool::remove(block_info::ptr block)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    const auto it = std::find(buffer_.begin(), buffer_.end(), block);

    if (it == buffer_.end())
    {
        mutex_.unlock_upgrade();
        //-----------------------------------------------------------------
        return;
    }

    const auto old_size = buffer_.size();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();
    buffer_.erase(it);
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    log::debug(LOG_BLOCKCHAIN)
        << "Orphan pool removed block [" << encode_hash(block->hash())
        << "] old size (" << old_size << "). with status: " << block->error().message();
}

// TODO: use hash table pool to eliminate this O(n^2) search.
void orphan_pool::filter(message::get_data::ptr message) const
{
    auto &inventories = message->inventories;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    for (auto it = inventories.begin(); it != inventories.end();)
        if (it->is_block_type() && exists(it->hash))
            it = inventories.erase(it);
        else
            ++it;
    ///////////////////////////////////////////////////////////////////////////
}

block_info::list orphan_pool::trace(block_info::ptr end) const
{
    block_info::list trace;
    trace.reserve(buffer_.size());
    trace.push_back(end);
    auto hash = end->actual()->header.previous_block_hash;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_shared();

    for (auto it = rfind(buffer_.rbegin(), hash); it != buffer_.rend(); it = rfind(it, hash))
    {
        trace.push_back(*it);
        hash = (*it)->actual()->header.previous_block_hash;
    }

    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    BITCOIN_ASSERT(!trace.empty());
    std::reverse(trace.begin(), trace.end());
    trace.shrink_to_fit();
    return trace;
}

block_info::list orphan_pool::unprocessed() const
{
    block_info::list unprocessed;
    unprocessed.reserve(buffer_.size());

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_shared();

    // Earlier blocks enter pool first, so reversal helps avoid fragmentation.
    for (auto it = buffer_.rbegin(); it != buffer_.rend(); ++it)
        if (!(*it)->processed())
            unprocessed.push_back(*it);

    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    unprocessed.shrink_to_fit();
    return unprocessed;
}

bool orphan_pool::add_pending_block(const hash_digest &needed_block, const block_info::ptr &pending_block)
{
    auto hash = pending_block->actual()->header.hash();
    if (pending_blocks_hash_.find(hash) != pending_blocks_hash_.end())
    {
        return false;
    }

    pending_blocks_.insert(make_pair(needed_block, pending_block));
    pending_blocks_hash_.insert(hash);
    return true;
}

block_info::ptr orphan_pool::delete_pending_block(const hash_digest &needed_block)
{
    block_info::ptr ret;
    auto it = pending_blocks_.find(needed_block);
    if (it != pending_blocks_.end())
    {
        ret = it->second;
        pending_blocks_hash_.erase(it->second->actual()->header.hash());
        pending_blocks_.erase(it);
    }

    return ret;
}

// private
//-----------------------------------------------------------------------------

bool orphan_pool::exists(const hash_digest &hash) const
{
    const auto match = [&hash](const block_info::ptr &entry) {
        return hash == entry->hash();
    };

    return std::any_of(buffer_.begin(), buffer_.end(), match);
}

bool orphan_pool::exists(const chain::header &header) const
{
    const auto match = [&header](const block_info::ptr &entry) {
        return header == entry->actual()->header;
    };

    return std::any_of(buffer_.begin(), buffer_.end(), match);
}

/*
orphan_pool::const_iterator orphan_pool::find(buffer::const_iterator begin, const hash_digest& hash) const
{
    const auto match = [&hash](const block_info::ptr& entry)
    {
        return hash == entry->hash();
    };

    return std::find_if(begin, buffer_.end(), match);
}
*/

orphan_pool::const_reverse_iterator orphan_pool::rfind(buffer::const_reverse_iterator begin, const hash_digest &hash) const
{
    const auto match = [&hash](const block_info::ptr &entry) {
        return hash == entry->hash();
    };

    return std::find_if(begin, buffer_.rend(), match);
}

} // namespace blockchain
} // namespace libbitcoin
