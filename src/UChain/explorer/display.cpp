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

#include <UChain/explorer/display.hpp>

#include <iostream>
#include <memory>
#include <boost/format.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChain/explorer/generated.hpp>
#include <UChain/explorer/utility.hpp>
#include <UChain/explorer/version.hpp>

namespace libbitcoin
{
namespace explorer
{

using namespace bc::config;

void display_command_names(std::ostream &stream)
{
    const auto func = [&stream](std::shared_ptr<command> explorer_command) {
        BITCOIN_ASSERT(explorer_command != nullptr);
        if (!explorer_command->obsolete())
            stream << "  " << explorer_command->name() << "\r\n";
    };

    broadcast(func, stream);
}

void display_connection_failure(std::ostream &stream, const endpoint &url)
{
    stream << format(BX_CONNECTION_FAILURE) % url;
}

void display_invalid_command(std::ostream &stream, const std::string &command,
                             const std::string &superseding)
{
    if (superseding.empty())
        stream << format(BX_INVALID_COMMAND) % command;
    else
        stream << format(BX_DEPRECATED_COMMAND) % command % superseding;
}

// English only hack to patch missing arg name in boost exception message.
static std::string fixup_boost_po_what_en(const std::string &what)
{
    std::string message(what);
    boost::replace_all(message, "for option is invalid", "is invalid");
    return message;
}

void display_invalid_parameter(std::ostream &stream,
                               const std::string &message)
{
    stream << format(BX_INVALID_PARAMETER) % fixup_boost_po_what_en(message);
}

void display_usage(std::ostream &stream)
{
    stream
        << std::endl
        << BX_COMMAND_USAGE << std::endl
        << format(BX_VERSION_MESSAGE) %
               UC_EXPLORER_VERSION
        << std::endl
        << BX_COMMANDS_HEADER << std::endl;

    display_command_names(stream);
}

} // namespace explorer
} // namespace libbitcoin
