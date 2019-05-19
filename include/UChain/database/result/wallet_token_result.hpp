/**
 * Copyright (c) 2018-2020 UChain developers 
 *
 * This file is part of uc-node.
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
#ifndef UC_DATABASE_WALLET_TOKEN_RESULT_HPP
#define UC_DATABASE_WALLET_TOKEN_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <UChain/coin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/result/base_result.hpp>
#include <UChainService/txs/token/token_transfer.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin
{
namespace database
{

/// read token detail information from token database.
class BCD_API wallet_token_result : public base_result
{
  public:
    wallet_token_result(const memory_ptr slab);

    /// The token wallet relationship.
    token_transfer get_wallet_token_transfer() const;
};

} // namespace database
} // namespace libbitcoin

#endif
