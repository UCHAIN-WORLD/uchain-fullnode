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

#include <UChain/explorer/commands/help.hpp>

#include <iostream>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/generated.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

console_result help::invoke(std::ostream &output, std::ostream &error)
{
    // Bound parameters.
    const auto &symbol = get_command_argument();

    if (symbol.empty())
    {
        display_usage(output);
        return console_result::okay;
    }

    auto command = find(symbol);
    if (!command)
    {
        display_invalid_command(error, symbol);
        return console_result::failure;
    }

    command->load_options();
    command->load_arguments();
    command->write_help(output);
    return console_result::okay;
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
