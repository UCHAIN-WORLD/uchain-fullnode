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
#include <UChainService/api/command/commands/showaddresses.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ showaddresses *************************/

console_result showaddresses::invoke(Json::Value &jv_output,
                                     libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    auto &aroot = jv_output;
    Json::Value addresses;

    auto vaddr = blockchain.get_wallet_addresses(auth_.name);
    if (!vaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

    for (auto &it : *vaddr)
    {
        addresses.append(it.get_address());
    }

    if (get_api_version() == 1 && addresses.isNull())
    { // compatible for v1
        aroot["addresses"] = "";
    }
    else if (get_api_version() <= 2)
    {
        aroot["addresses"] = addresses;
    }
    else
    {
        if (addresses.isNull())
            addresses.resize(0);

        aroot = addresses;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
