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
#include <UChain/blockchain/block_info.hpp>

#include <cstdint>
#include <memory>
#include <utility>
#include <UChain/coin.hpp>

namespace libbitcoin
{
namespace blockchain
{

using namespace message;

// Use zero as orphan sentinel since this is precluded by the orphan pool.
static constexpr auto orphan_height = 0u;

block_info::block_info(block_ptr actual_block)
    : code_(error::success),
      processed_(false),
      height_(orphan_height),
      actual_block_(actual_block),
      is_checked_work_proof_(false)
{
}

// Hand off ownership of a block to this wrapper.
block_info::block_info(chain::block &&actual_block)
    : block_info(std::make_shared<block_msg>(actual_block))
{
}

block_info::block_ptr block_info::actual() const
{
    return actual_block_;
}

void block_info::set_processed()
{
    processed_.store(true);
}

bool block_info::processed() const
{
    return processed_.load();
}

void block_info::set_height(uint64_t height)
{
    BITCOIN_ASSERT(height != orphan_height);
    height_.store(height);
}

uint64_t block_info::height() const
{
    return height_.load();
}

void block_info::set_is_checked_work_proof(bool is_checked)
{
    is_checked_work_proof_.store(is_checked);
}

bool block_info::get_is_checked_work_proof() const
{
    return is_checked_work_proof_.load();
}

void block_info::set_error(const code &code)
{
    code_.store(code);
}

code block_info::error() const
{
    return code_.load();
}

const hash_digest block_info::hash() const
{
    // This relies on the hash caching optimization.
    return actual_block_->header.hash();
}

} // namespace blockchain
} // namespace libbitcoin
