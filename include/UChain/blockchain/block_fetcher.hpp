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
#ifndef UC_BLOCKCHAIN_BLOCK_FETCHER_HPP
#define UC_BLOCKCHAIN_BLOCK_FETCHER_HPP

#include <cstdint>
#include <memory>
#include <system_error>
#include <UChain/coin.hpp>
#include <UChain/blockchain/define.hpp>
#include <UChain/blockchain/block_chain.hpp>

namespace libbitcoin
{
namespace blockchain
{

/**
 * Fetch a block by height.
 *
 * If the blockchain reorganises this call may fail.
 *
 * @param[in]   chain           Blockchain service
 * @param[in]   height          Height of block to fetch.
 * @param[in]   handle_fetch    Completion handler for fetch operation.
 */
BCB_API void fetch_block(block_chain &chain, uint64_t height,
                         block_chain::block_fetch_handler handle_fetch);

BCB_API void fetch_latest_transactions(block_chain &chain, uint64_t height,
                                       uint32_t index, uint32_t count, block_chain::transactions_fetch_handler handle_fetch);
/**
 * Fetch a block by hash.
 *
 * If the blockchain reorganises this call may fail.
 *
 * @param[in]   chain           Blockchain service
 * @param[in]   hash            Block hash
 * @param[in]   handle_fetch    Completion handler for fetch operation.
 */
BCB_API void fetch_block(block_chain &chain, const hash_digest &hash,
                         block_chain::block_fetch_handler handle_fetch);

} // namespace blockchain
} // namespace libbitcoin

#endif
