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
#include <UChain/database/result/wallet_token_result.hpp>
#include <UChainService/txs/token/token_transfer.hpp>
#include <cstddef>
#include <cstdint>
#include <UChain/coin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin
{
namespace database
{

template <typename Iterator>
token_transfer deserialize_token_transfer(const Iterator first)
{
    token_transfer sp_detail;
    auto deserial = make_deserializer_unsafe(first);
    sp_detail.from_data(deserial);
    return sp_detail;
}

wallet_token_result::wallet_token_result(const memory_ptr slab)
    : base_result(slab)
{
}

token_transfer wallet_token_result::get_wallet_token_transfer() const
{
    BITCOIN_ASSERT(get_slab());
    const auto memory = REMAP_ADDRESS(get_slab());
    return deserialize_token_transfer(memory);
}
} // namespace database
} // namespace libbitcoin
