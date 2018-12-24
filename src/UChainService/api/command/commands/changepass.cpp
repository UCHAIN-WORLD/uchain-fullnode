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


#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/changepass.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result changepass::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);

    acc->set_passwd(option_.passwd);
    acc->set_mnemonic(mnemonic, option_.passwd);

    blockchain.store_wallet(acc);

    // reencry address
    auto pvaddr = blockchain.get_wallet_addresses(auth_.name);
    if(!pvaddr)
        throw address_list_nullptr_exception{"empty address list"};

    std::string prv_key;
    for (auto& each : *pvaddr){
        prv_key = each.get_prv_key(auth_.auth);
        each.set_prv_key(prv_key, option_.passwd);
    }
    // delete all old address
    blockchain.delete_wallet_address(auth_.name);
    // restore address
    for (auto& each : *pvaddr) {
        auto addr = std::make_shared<bc::chain::wallet_address>(each);
        blockchain.store_wallet_address(addr);
    }

    auto& jv = jv_output;
    jv["name"] = auth_.name;
    jv["status"] = "changed successfully";

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

