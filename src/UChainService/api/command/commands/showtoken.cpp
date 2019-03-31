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
#include <UChainService/api/command/commands/showtoken.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showtoken *************************/

console_result showtoken::invoke(Json::Value &jv_output,
                                 libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();

    if (!argument_.symbol.empty())
    {
        // check token symbol
        blockchain.uppercase_symbol(argument_.symbol);
        check_token_symbol(argument_.symbol);
    }

    std::string json_key;
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    if (option_.is_cert)
    { // only get token certs
        json_key = "tokencerts";

        // get token cert in blockchain
        auto sh_vec = blockchain.get_issued_token_certs();
        if (argument_.symbol.empty())
        {
            std::set<std::string> symbols;
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto &elem : *sh_vec)
            {
                // get rid of duplicate symbols
                if (!symbols.count(elem.get_symbol()))
                {
                    symbols.insert(elem.get_symbol());
                    json_value.append(elem.get_symbol());
                }
            }
        }
        else
        {
            auto result_vec = std::make_shared<token_cert::list>();
            for (auto &cert : *sh_vec)
            {
                if (argument_.symbol != cert.get_symbol())
                {
                    continue;
                }

                result_vec->push_back(cert);
            }

            std::sort(result_vec->begin(), result_vec->end());
            for (auto &elem : *result_vec)
            {
                Json::Value token_data = json_helper.prop_list(elem);
                json_value.append(token_data);
            }
        }
    }
    else
    {
        json_key = "tokens";

        // get token in blockchain
        auto sh_vec = blockchain.get_issued_tokens(argument_.symbol);
        if (argument_.symbol.empty())
        {
            std::sort(sh_vec->begin(), sh_vec->end());
            std::set<std::string> symbols;
            for (auto &elem : *sh_vec)
            {
                // get rid of duplicate symbols
                if (!symbols.count(elem.get_symbol()))
                {
                    symbols.insert(elem.get_symbol());
                    json_value.append(elem.get_symbol());
                }
            }
        }
        else
        {
            for (auto &elem : *sh_vec)
            {
                if (elem.get_symbol() != argument_.symbol)
                {
                    continue;
                }

                Json::Value token_data = json_helper.prop_list(elem, true);
                token_data["status"] = "issued";
                json_value.append(token_data);
            }
        }
    }

    if (get_api_version() == 1 && json_value.isNull())
    { //compatible for v1
        jv_output[json_key] = "";
    }
    else if (get_api_version() <= 2)
    {
        jv_output[json_key] = json_value;
    }
    else
    {
        if (json_value.isNull())
            json_value.resize(0);

        jv_output = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
