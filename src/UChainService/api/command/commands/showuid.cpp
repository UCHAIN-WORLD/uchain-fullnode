/**
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#include <UChainService/api/command/commands/showuid.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result showuid::invoke (Json::Value& jv_output,
                               libbitcoin::server::server_node& node)
{
    Json::Value json_value;

    auto& blockchain = node.chain_impl();

    if (option_.symbol.empty()) {

        auto sh_vec = blockchain.get_registered_uids();

        std::sort(sh_vec->begin(), sh_vec->end());
        // add blockchain uids
        for (auto& elem : *sh_vec) {
            json_value.append(elem.get_symbol());
        }

        if (get_api_version() <= 2) {
            jv_output["uids"] = json_value;
        }
        else {
            jv_output = json_value;
        }
    }
    else {
        auto uidSymbol = option_.symbol;
        if (blockchain.is_valid_address(uidSymbol)) {
            uidSymbol = blockchain.get_uid_from_address(uidSymbol);
            if (uidSymbol.empty()) {
                throw address_not_bound_uid_exception{"address is not binded with some uid on the blockchain"};
            }
        }

        // check uid symbol
        check_uid_symbol(uidSymbol);

        // check uid exists
        if (!blockchain.is_uid_exist(uidSymbol)) {
            throw uid_symbol_notfound_exception{"uid symbol does not exist on the blockchain"};
        }

        auto blockchain_uids = blockchain.get_uid_history_addresses(uidSymbol);
        if (blockchain_uids) {
            Json::Value json_address;
            Json::Value uid_data;
            for (auto &uid : *blockchain_uids) {
                uid_data["address"] = uid.get_uid().get_address();
                uid_data["status"] = uid.get_status_string();
                if (get_api_version() >= 3) {
                    uid_data["symbol"] = uidSymbol;
                }
                json_value.append(uid_data);
            }
            
            if (get_api_version() <= 2) {
                jv_output["uid"] = uidSymbol;
                jv_output["addresses"] = json_value;
            }
            else {
                if(json_value.isNull())
                    json_value.resize(0);

                jv_output = json_value;
            }
        }
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

