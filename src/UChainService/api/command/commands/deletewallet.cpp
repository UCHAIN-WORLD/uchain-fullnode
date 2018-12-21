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
#include <UChainService/api/command/commands/deletewallet.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ deletewallet *************************/

console_result deletewallet::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    blockchain.is_wallet_lastwd_valid(*acc, auth_.auth, argument_.last_word);

    // delete wallet addresses
    blockchain.delete_wallet_address(acc->get_name());

    // delete wallet token
    blockchain.delete_wallet_token(acc->get_name());
    // delete wallet
    blockchain.delete_wallet(acc->get_name());

    jv_output["name"] = acc->get_name();
    jv_output["status"]= "removed successfully";

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

