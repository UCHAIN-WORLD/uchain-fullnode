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
#include <UChainService/api/command/commands/deposit.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result deposit::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    if (argument_.deposit != 10 && argument_.deposit != 45
        && argument_.deposit != 120 && argument_.deposit != 240
        && argument_.deposit != 540)
    {
        throw wallet_deposit_period_exception{"deposit must be one in [10, 45, 120, 240, 540]."};
    }

    auto pvaddr = blockchain.get_wallet_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty())
        throw address_list_nullptr_exception{"nullptr for address list"};

    std::string addr = argument_.uid;
    if (addr.empty()) {
        addr = get_random_payment_address(pvaddr, blockchain);
    }else{
        addr = get_address_from_strict_uid(argument_.uid, blockchain);
        auto acc_addr = blockchain.get_wallet_address(auth_.name, addr);
        if (!acc_addr)
            throw argument_legality_exception{"You don't own address " + addr};
    }

    // receiver
    std::vector<receiver_record> receiver{
        {addr, "", argument_.amount, 0, utxo_attach_type::deposit, asset{"", argument_.uid}}
    };
    auto deposit_helper = depositing_ucn(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(addr), std::move(receiver), argument_.deposit, argument_.fee);

    deposit_helper.exec();

    // json output
    auto tx = deposit_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

