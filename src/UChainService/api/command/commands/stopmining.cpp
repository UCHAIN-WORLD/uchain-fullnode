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

#include <jsoncpp/json/json.h>
#include <UChainService/api/command/node_method_wrapper.hpp>
#include <UChainService/api/command/commands/stopmining.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/************************ stopmining *************************/

console_result stopmining::invoke(Json::Value &jv_output,
                                  libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    auto &miner = node.miner();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);
    if (!administrator_required_checker(node, auth_.name, auth_.auth) &&
        !blockchain.get_wallet_address(auth_.name, miner.get_miner_address()))
        throw wallet_authority_exception{"not the miner wallet."};
    auto ret = miner.stop();

    if (ret)
    {
        jv_output = "signal STOP sent.";
    }
    else
    {
        throw unknown_error_exception{"mining stop got error."};
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
