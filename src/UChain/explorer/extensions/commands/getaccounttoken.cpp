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

#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChain/explorer/extensions/commands/getaccounttoken.hpp>
#include <UChain/explorer/extensions/command_extension_func.hpp>
#include <UChain/explorer/extensions/command_assistant.hpp>
#include <UChain/explorer/extensions/exception.hpp>
#include <UChain/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaccounttoken *************************/

console_result getaccounttoken::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (!argument_.symbol.empty()) {
        // check token symbol
        check_token_symbol(argument_.symbol);
    }

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

    std::string json_key;
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    if (option_.is_cert) { // only get token certs
        json_key = "tokencerts";
        auto sh_vec = std::make_shared<token_cert::list>();
        for (auto& each : *pvaddr){
            sync_fetch_token_cert_balance(each.get_address(), argument_.symbol, blockchain, sh_vec);
        }

        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            if (!argument_.symbol.empty() && argument_.symbol != elem.get_symbol())
                continue;

            Json::Value token_cert = json_helper.prop_list(elem);
            json_value.append(token_cert);
        }
    }
    else if (option_.deposited) {
        json_key = "tokens";
        auto sh_vec = std::make_shared<token_deposited_balance::list>();

        // get address unspent token balance
        std::string addr;
        for (auto& each : *pvaddr){
            sync_fetch_token_deposited_balance(each.get_address(), blockchain, sh_vec);
        }

        std::sort(sh_vec->begin(), sh_vec->end());

        for (auto& elem: *sh_vec) {
            auto& symbol = elem.symbol;
            if (!argument_.symbol.empty() && argument_.symbol != symbol)
                continue;

            auto issued_token = blockchain.get_issued_token(symbol);
            if (!issued_token) {
                continue;
            }

            Json::Value token_data = json_helper.prop_list(elem, *issued_token, true);
            token_data["status"] = "unspent";
            json_value.append(token_data);
        }
    }
    else {
        json_key = "tokens";
        auto sh_vec = std::make_shared<token_balances::list>();

        // 1. get token in blockchain
        // get address unspent token balance
        std::string addr;
        for (auto& each : *pvaddr){
            sync_fetch_token_balance(each.get_address(), false, blockchain, sh_vec);
        }

        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            auto& symbol = elem.symbol;
            if (!argument_.symbol.empty() && argument_.symbol != symbol)
                continue;
            auto issued_token = blockchain.get_issued_token(symbol);
            if (!issued_token) {
                continue;
            }
            Json::Value token_data = json_helper.prop_list(elem, *issued_token);
            token_data["status"] = "unspent";
            json_value.append(token_data);
        }

        // 2. get token in local database
        // shoudl filter all issued token which be stored in local account token database
        auto sh_unissued = blockchain.get_account_unissued_tokens(auth_.name);
        for (auto& elem: *sh_unissued) {
            auto& symbol = elem.detail.get_symbol();
            // symbol filter
            if(!argument_.symbol.empty() && argument_.symbol !=  symbol)
                continue;

            Json::Value token_data = json_helper.prop_list(elem.detail, false);
            token_data["status"] = "unissued";
            json_value.append(token_data);
        }
    }

    if (get_api_version() == 1 && json_value.isNull()) { //compatible for v1
        jv_output[json_key] = "";
    }
    else if (get_api_version() <= 2) {
        jv_output[json_key] = json_value;
    }
    else {
        if (json_value.isNull())
            json_value.resize(0);

        jv_output = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

