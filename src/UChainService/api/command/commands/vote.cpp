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
#include <UChainService/api/command/commands/vote.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result vote::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!argument_.from.empty() && !blockchain.is_valid_address(argument_.from))
        throw address_invalid_exception{"invalid from address!"};
    if(!argument_.to.empty() && !blockchain.is_valid_address(argument_.to))
        throw address_invalid_exception{"invalid to address!"};


    if (argument_.amount <= 0) {
        throw argument_legality_exception("invalid amount parameter!");
    }

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.from, "", argument_.amount*coin_price(1)/20000, 0, utxo_attach_type::deposit, asset()},
        {argument_.to, UC_VOTE_TOKEN_SYMBOL, 0, argument_.amount, utxo_attach_type::token_transfer, asset()}
    };
    auto vote_helper = voting_token(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(argument_.from), std::move(receiver), argument_.amount, option_.fee);

    vote_helper.exec();

    // json output
    auto tx = vote_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

