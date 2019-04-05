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

#include <UChainService/api/command/commands/shutdown.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/node_method_wrapper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/************************ shutdown *************************/
console_result shutdown::invoke(Json::Value &jv_output,
                                libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();

    administrator_required_checker(node, auth_.name, auth_.auth);
    jv_output = "sending SIGTERM to ucd.";

#ifndef _WIN32
    killpg(getpgrp(), SIGTERM);
#else
    std::thread(
        []() {
            Sleep(2000);
            ExitProcess(0);
        })
        .detach();
#endif

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
