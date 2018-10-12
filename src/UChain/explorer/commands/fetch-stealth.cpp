/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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

#include <UChain/explorer/commands/fetch-stealth.hpp>

#include <iostream>
#include <UChain/client.hpp>
#include <UChain/explorer/callback_state.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/json_helper.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::chain;
using namespace bc::client;
using namespace bc::explorer::config;
using namespace bc::wallet;

console_result fetch_stealth::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto height = get_height_option();
    const auto& encoding = get_format_option();
    const auto& filter = get_filter_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    if (filter.size() > stealth_address::max_filter_bits)
    {
        error << BX_FETCH_STEALTH_FILTER_TOO_LONG << std::flush;
        return console_result::failure;
    }

    callback_state state(error, output, encoding);

    // This enables json-style array formatting.
    const auto json = encoding == encoding_engine::json;

    auto on_done = [&state, json](const stealth::list& list)
    {
        // Write out the transaction hashes of *potential* matches.
        state.output(json_helper().prop_tree(list, json));
    };

    auto on_error = [&state](const std::error_code& error)
    {
        state.succeeded(error);
    };

    client.blockchain_fetch_stealth(on_error, on_done, filter, height);
    client.wait();

    return state.get_result();
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
