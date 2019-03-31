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
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/showminers.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/consensus/miner.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showminers *************************/

console_result showminers::invoke(Json::Value &jv_output,
                                  libbitcoin::server::server_node &node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    auto &aroot = jv_output;
    Json::Value miners;
    auto json_helper = config::json_helper(get_api_version());

    auto sh_vec = node.miner().get_miners();
    for (auto &elem : sh_vec)
    {
        Json::Value token_data = json_helper.prop_list(elem, true, true);
        miners.append(token_data);
    }

    if (miners.isNull())
        miners.resize(0);

    aroot = miners;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
