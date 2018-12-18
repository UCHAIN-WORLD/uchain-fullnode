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
    if(!argument_.from.empty() && !chain::output::is_valid_uid_symbol(argument_.from))
        throw uid_symbol_name_exception{"Did symbol " + argument_.from + " is not valid."};

    std::vector<receiver_record> receiver{
    };
    uint64_t amount = 0;
    for (auto& each : argument_.to) {
        colon_delimited2_item<std::string, uint64_t> item(each);

        asset attach;
        std::string address = get_address(item.first(), attach, false, blockchain);
        if (item.second() <= 0) {
            throw argument_legality_exception("invalid amount parameter for " + item.first());
        }
        attach.set_from_uid(argument_.from);
        amount += item.second();
        receiver.push_back({address, UC_VOTE_TOKEN_SYMBOL, 0, item.second(), utxo_attach_type::token_transfer, attach});
    }


    asset f_attach;
    std::string from_address = get_address(argument_.from, f_attach, false, blockchain);
    receiver.push_back({from_address, "", amount*coin_price(1)/20, 0, utxo_attach_type::deposit, f_attach});

    auto vote_helper = voting_token(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(from_address), std::move(receiver), amount, option_.fee);

    vote_helper.exec();

    // json output
    auto tx = vote_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

