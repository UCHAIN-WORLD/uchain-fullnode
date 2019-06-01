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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChain/coin/constants.hpp>

namespace libbitcoin
{
uint32_t coinbase_maturity = 1;

hash_number max_target()
{
    hash_number max_target;
    max_target.set_compact(max_work_bits);
    return max_target;
}

std::string get_developer_community_address(bool is_testnet)
{
    std::string address("UNKAn2fsG5CPeP4s9mxTvJKSjmqqdwu3Tq"); // developers address for mainnet
    if (is_testnet)
    {
        address = "tJNo92g6DavpaCZbYjrH45iQ8eAKnLqmms"; // developers address for testnet
    }
    return address;
}

std::string get_foundation_address(bool is_testnet)
{
    std::string address("UNfrtAxhJRi83PjTPjV3yNPKnjLYR22Bhx"); // foundation address for mainnet
    if (is_testnet)
    {
        address = "tFzJJnso5tKDdwTiztMq1qMg1uHfqbbpq6"; // foundation address for testnet
    }
    return address;
}

std::string get_reward_pool_address(bool is_testnet)
{
    std::string address("UkwAarnHiVUoBi4mHupZ3GyLcVLDxPe9eo"); // foundation address for mainnet
    if (is_testnet)
    {
        address = "tFzJJnso5tKDdwTiztMq1qMg1uHfqbbpq6"; // foundation address for testnet
    }
    return address;
}

} // namespace libbitcoin
