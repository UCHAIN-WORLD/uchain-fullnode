/*
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
#ifndef UC_BLOCKCHAIN_BLOCK_LOCATOR_INDEXES_HPP
#define UC_BLOCKCHAIN_BLOCK_LOCATOR_INDEXES_HPP

#include <cstdint>
#include <UChain/bitcoin.hpp>
#include <UChain/blockchain/define.hpp>
#include <UChain/bitcoin/chain/header.hpp>

namespace libbitcoin {
namespace blockchain {

BCB_API u256 block_work(u256 bits);

BCB_API chain::block::indexes block_locator_indexes(size_t top_height);

} // namespace blockchain
} // namespace libbitcoin

#endif
