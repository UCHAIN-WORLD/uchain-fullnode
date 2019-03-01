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
#include <UChain/database/result/token_result.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <cstddef>
#include <cstdint>
#include <UChain/bitcoin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin
{
namespace database
{

template <typename Iterator>
std::shared_ptr<token_detail> deserialize_wallet_detail(const Iterator first)
{
    auto detail = std::make_shared<token_detail>();
    auto deserial = make_deserializer_unsafe(first);
    detail->from_data(deserial);
    return detail;
}
token_result::token_result(const memory_ptr slab)
    : base_result(slab)
{
}

std::shared_ptr<token_detail> token_result::get_token_detail() const
{
    //BITCOIN_ASSERT(get_slab());
    std::shared_ptr<token_detail> sp_acc(nullptr);
    if (get_slab())
    {
        const auto memory = REMAP_ADDRESS(get_slab());
        sp_acc = deserialize_wallet_detail(memory);
    }
    return sp_acc;
}

} // namespace database
} // namespace libbitcoin
