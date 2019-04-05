/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
 *
 * UChain-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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

#include <UChainService/api/command/node_method_wrapper.hpp>
#include <UChainService/api/command/commands/showwork.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showwork *************************/

console_result showwork::invoke(Json::Value &jv_output,
                                libbitcoin::server::server_node &node)
{

    administrator_required_checker(node, auth_.name, auth_.auth);

    std::string seed_hash;
    std::string header_hash;
    std::string boundary;

    auto &blockchain = node.chain_impl();
    auto &miner = node.miner();

    //auto ret = miner.get_work(seed_hash, header_hash, boundary);
    auto ret = 0;

    auto &aroot = jv_output;

    if (ret)
    {

        Json::Value result;
        result.append(header_hash);
        result.append(seed_hash);
        result.append(boundary);

        if (get_api_version() == 1)
        {
            aroot["result"] = result;
        }
        else
        {
            aroot = result;
        }
    }
    else
    {
        throw setting_required_exception{"Use command <setminingwallet> to set mining address."};
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
