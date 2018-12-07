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
#include <UChainService/api/command/commands/showtokens.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/base_helper.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ showtokens *************************/

console_result showtokens::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    std::string json_key;
    Json::Value json_value;
    
    auto json_helper = config::json_helper(get_api_version());

    if (option_.is_cert) { // only get token certs
        json_key = "tokencerts";

        if (auth_.name.empty()) { // no account -- list whole token certs in blockchain
            auto result_vec = blockchain.get_issued_token_certs();
            std::sort(result_vec->begin(), result_vec->end());
            for (auto& elem : *result_vec) {
                Json::Value token_data = json_helper.prop_list(elem);
                json_value.append(token_data);
            }
        }
        else { // list token certs owned by account
            blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if (!pvaddr)
                throw address_list_nullptr_exception{"nullptr for address list"};

            auto sh_vec = std::make_shared<token_cert::list>();
            for (auto& each : *pvaddr) {
                sync_fetch_token_cert_balance(each.get_address(), "", blockchain, sh_vec);
            }

            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                Json::Value token_cert = json_helper.prop_list(elem);
                json_value.append(token_cert);
            }
        }
    }
    else {
        json_key = "tokens";

        if (auth_.name.empty()) { // no account -- list whole tokens in blockchain
            auto sh_vec = blockchain.get_issued_tokens();
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                Json::Value token_data = json_helper.prop_list(elem, true);
                token_data["status"] = "issued";
                json_value.append(token_data);
            }
        }
        else { // list token owned by account
            blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if (!pvaddr)
                throw address_list_nullptr_exception{"nullptr for address list"};

            auto sh_vec = std::make_shared<token_balances::list>();

            // 1. get token in blockchain
            // get address unspent token balance
            for (auto& each : *pvaddr) {
                sync_fetch_token_balance(each.get_address(), true, blockchain, sh_vec);
            }

            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                auto issued_token = blockchain.get_issued_token(elem.symbol);
                if (!issued_token) {
                    continue;
                }
                Json::Value token_data = json_helper.prop_list(elem, *issued_token, false);
                token_data["status"] = "unspent";
                json_value.append(token_data);
            }

            // 2. get token in local database
            // shoudl filter all issued token which be stored in local account token database
            sh_vec->clear();
            auto sh_unissued = blockchain.get_account_unissued_tokens(auth_.name);
            for (auto& elem: *sh_unissued) {
                Json::Value token_data = json_helper.prop_list(elem.detail, false, false);
                token_data["status"] = "unissued";
                json_value.append(token_data);
            }
        }
    }

    if (get_api_version() == 1 && json_value.isNull()) { //compatible for v1
        jv_output[json_key] = "";
    }
    else if (get_api_version() <= 2) {
        jv_output[json_key] = json_value;
    }
    else {
        if(json_value.isNull())
            json_value.resize(0);  
            
        jv_output = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
