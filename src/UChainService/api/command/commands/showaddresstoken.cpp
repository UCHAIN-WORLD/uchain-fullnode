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
#include <UChainService/api/command/commands/showaddresstoken.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ showaddresstoken *************************/

console_result showaddresstoken::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address!"};

    if (!option_.symbol.empty()) {
        // check token symbol
        check_token_symbol(option_.symbol);
    }

    std::string json_key;
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());;

    if (option_.is_cert) { // only get token certs
        json_key = "tokencerts";

        auto sh_vec = std::make_shared<token_cert::list>();
        sync_fetch_token_cert_balance(argument_.address, "", blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.get_symbol())
                continue;

            Json::Value token_cert = json_helper.prop_list(elem);
            json_value.append(token_cert);
        }
    }
    else if (option_.deposited) {
        json_key = "tokens";

        auto sh_vec = std::make_shared<token_deposited_balance::list>();
        sync_fetch_token_deposited_balance(argument_.address, blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());

        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.symbol)
                continue;

            auto issued_token = blockchain.get_issued_token(elem.symbol);
            if (!issued_token) {
                continue;
            }

            Json::Value token_data = json_helper.prop_list(elem, *issued_token);
            token_data["status"] = "unspent";
            json_value.append(token_data);
        }
    }
    else {
        json_key = "tokens";

        auto sh_vec = std::make_shared<token_balances::list>();
        sync_fetch_token_balance(argument_.address, true, blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.symbol)
                continue;

            auto issued_token = blockchain.get_issued_token(elem.symbol);
            if (!issued_token) {
                continue;
            }
            Json::Value token_data = json_helper.prop_list(elem, *issued_token);
            token_data["status"] = "unspent";
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
        if(json_value.isNull())
            json_value.resize(0);

        jv_output = json_value;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

