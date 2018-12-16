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


#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/addpeer.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/node_method_wrapper.hpp>
#include <UChain/bitcoin/config/authority.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChain/network/channel.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ addpeer *************************/

console_result addpeer::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    administrator_required_checker(node, auth_.name, auth_.auth);
    try
    {
        const auto authority = libbitcoin::config::authority(argument_.address);

        code errcode;
        auto handler = [&errcode](const code &ec) {
            errcode = ec;
        };

        if (option_.operation == "ban")
        {
            network::channel::manual_ban(authority);
            node.connections_ptr()->stop(authority);
        }
        else if ((option_.operation == "add") || (option_.operation == ""))
        {
            network::channel::manual_unban(authority);
            node.store(authority.to_network_address(), handler);
        }
        else
        {
            jv_output = string("Invalid operation [") + option_.operation + "].";
            return console_result::okay;
        }
        jv_output = errcode.message();
    }
    catch (...)
    {
        throw address_invalid_exception{"NODEADDRESS is not valid! "};
    }

    
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

