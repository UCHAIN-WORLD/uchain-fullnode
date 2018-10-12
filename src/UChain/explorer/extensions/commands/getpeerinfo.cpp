/**
 * Copyright (c) 2018-2020 uc developers
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


#include <UChain/node/p2p_node.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChain/explorer/extensions/commands/getpeerinfo.hpp>
#include <UChain/explorer/extensions/command_extension_func.hpp>
#include <UChain/explorer/extensions/command_assistant.hpp>
#include <UChain/explorer/extensions/node_method_wrapper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getpeerinfo *************************/

console_result getpeerinfo::invoke(Json::Value& jv_output,
                                   libbitcoin::server::server_node& node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    Json::Value array;
    for (auto authority : node.connections_ptr()->authority_list()) {
        // invalid authority
        if (authority.to_hostname() == "[::]" && authority.port() == 0)
            continue;
        array.append(authority.to_string());
    }

    if (get_api_version() <= 2) {
        auto& root = jv_output;
        root["peers"] = array;
    }
    else {
        if(array.isNull())
            array.resize(0);  
            
        jv_output = array;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

