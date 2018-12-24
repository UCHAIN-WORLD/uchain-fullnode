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
#include <UChainService/api/command/commands/showvote.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showvote *************************/

console_result showvote::invoke(Json::Value &jv_output,
                                libbitcoin::server::server_node &node)
{
    if (argument_.endheight < argument_.startheight)
    {
        throw block_height_exception{"END_HEIGHT is less than START_HEIGHT."};
    }

    auto &blockchain = node.chain_impl();
    std::string from_address = get_address_from_strict_uid(argument_.uid, blockchain);

    if (!blockchain.is_valid_address(from_address))
        throw uid_symbol_name_exception{"Uid symbol " + argument_.uid + " is not valid."};

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    auto sh_vec = std::make_shared<token_balances::list>();
    sync_fetch_token_balance(from_address, true, blockchain, sh_vec);
    std::sort(sh_vec->begin(), sh_vec->end());
    for (auto &elem : *sh_vec)
    {
        if ( elem.symbol != UC_VOTE_TOKEN_SYMBOL)
            continue;

        auto issued_token = blockchain.get_issued_token(UC_VOTE_TOKEN_SYMBOL);
        if (!issued_token)
        {
            continue;
        }
        
        Json::Value token_data = json_helper.prop_list(elem, *issued_token);
        token_data["status"] = "unspent";
        json_value.append(token_data);
    }

    if (json_value.isNull())
        json_value.resize(0);

    jv_output = json_value;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
