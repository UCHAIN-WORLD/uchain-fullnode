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

#include <UChain/explorer/json_helper.hpp>
#include <UChainService/api/command/node_method_wrapper.hpp>
#include <UChainService/api/command/commands/showmininginfo.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showmininginfo *************************/

console_result showmininginfo::invoke(Json::Value &jv_output,
                                      libbitcoin::server::server_node &node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    uint64_t height, rate;
    std::string difficulty;
    bool is_mining;

    auto &miner = node.miner();
    //miner.get_state(height, rate, difficulty, is_mining);

    if (get_api_version() <= 2)
    {
        auto &aroot = jv_output;
        Json::Value info;
        info["is-mining"] = is_mining;
        info["height"] += height;
        info["rate"] += rate;
        info["difficulty"] = difficulty;
        aroot["mining-info"] = info;
    }
    else
    {
        jv_output["is_mining"] = is_mining;
        jv_output["height"] += height;
        jv_output["rate"] += rate;
        jv_output["difficulty"] = difficulty;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
