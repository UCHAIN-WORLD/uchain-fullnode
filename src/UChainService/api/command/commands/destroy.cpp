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
#include <UChainService/api/command/commands/destroy.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result destroy::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    std::string blackhole_uid = uid_detail::get_blackhole_uid_symbol();

    if (option_.is_candidate) {
        const char* cmds[] {
            "transfercandidate", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_uid.c_str(), argument_.symbol.c_str()
        };

        return dispatch_command(5, cmds, jv_output, node, get_api_version());
    }
    else if (!option_.cert_type.empty()) {
        const char* cmds[] {
            "transfercert", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_uid.c_str(), argument_.symbol.c_str(), option_.cert_type.c_str()
        };

        return dispatch_command(6, cmds, jv_output, node, get_api_version());
    }
    else {
        if (argument_.amount <= 0) {
            throw argument_legality_exception{"invalid amount parameter!"};
        }

        auto&& amount = std::to_string(argument_.amount);
        const char* cmds[] {
            "sendtokento", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_uid.c_str(), argument_.symbol.c_str(), amount.c_str()
        };

        return dispatch_command(6, cmds, jv_output, node, get_api_version());
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

