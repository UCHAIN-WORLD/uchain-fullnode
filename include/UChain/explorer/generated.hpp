/**
 * Copyright (c) 2011-2018 libbitcoin developers 
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
#ifndef BX_GENERATED_HPP
#define BX_GENERATED_HPP

#include <functional>
#include <memory>
#include <string>
#include <UChain/explorer/command.hpp>
#include <UChain/explorer/commands/fetch-history.hpp>
#include <UChain/explorer/commands/fetch-stealth.hpp>
#include <UChain/explorer/commands/help.hpp>
#include <UChain/explorer/commands/send-tx.hpp>
#include <UChain/explorer/commands/settings.hpp>
#include <UChain/explorer/commands/stealth-decode.hpp>
#include <UChain/explorer/commands/stealth-encode.hpp>
#include <UChain/explorer/commands/stealth-public.hpp>
#include <UChain/explorer/commands/stealth-secret.hpp>
#include <UChain/explorer/commands/stealth-shared.hpp>
#include <UChain/explorer/commands/tx-decode.hpp>
#include <UChain/explorer/commands/validate-tx.hpp>
#include <UChain/explorer/define.hpp>

/********* GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY **********/

namespace libbitcoin
{
namespace explorer
{

/**
 * Various shared localizable strings.
 */
#define BX_COMMANDS_HEADER \
    "Info: The commands are:"
#define BX_COMMANDS_HOME_PAGE \
    "UC home page:"
#define BX_COMMAND_USAGE \
    "Usage: help COMMAND"
#define BX_CONFIG_DESCRIPTION \
    "The path to the configuration settings file."
#define BX_CONNECTION_FAILURE \
    "Could not connect to server: %1%"
#define BX_DEPRECATED_COMMAND \
    "The '%1%' command has been replaced by '%2%'."
#define BX_HELP_DESCRIPTION \
    "Get a description and instructions for this command."
#define BX_INVALID_COMMAND \
    "'%1%' is not a command. Enter 'help' for a list of commands."
#define BX_INVALID_PARAMETER \
    "%1%"
#define BX_PRINTER_ARGUMENT_TABLE_HEADER \
    "Arguments (positional):"
#define BX_PRINTER_DESCRIPTION_FORMAT \
    "Info: %1%"
#define BX_PRINTER_OPTION_TABLE_HEADER \
    "Options (named):"
#define BX_PRINTER_USAGE_FORMAT \
    "Usage: %1% %2% %3%"
#define BX_PRINTER_VALUE_TEXT \
    "VALUE"
#define BX_VERSION_MESSAGE \
    "Version: %1%"

/**
 * Invoke a specified function on all commands.
 * @param[in]  func  The function to invoke on all commands.
 */
void broadcast(const std::function<void(std::shared_ptr<command>)> func, std::ostream &os);

/**
 * Find the command identified by the specified symbolic command name.
 * @param[in]  symbol  The symbolic command name.
 * @return             An instance of the command or nullptr if not found.
 */
std::shared_ptr<command> find(const std::string &symbol);

/**
 * Find the new name of the formerly-named command.
 * @param[in]  former  The former symbolic command name.
 * @return             The current name of the formerly-named command.
 */
std::string formerly(const std::string &former);

} // namespace explorer
} // namespace libbitcoin

#endif
