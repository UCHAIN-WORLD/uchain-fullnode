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

#include <UChain/explorer/commands/fetch-history.hpp>

#include <iostream>
#include <UChain/client.hpp>
#include <UChain/explorer/callback_state.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/json_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::chain;
using namespace bc::client;
using namespace bc::explorer::config;

// When you restore your wallet, you should use fetch_history().
// But for updating the wallet, use the [new] scan() method-
// which is faster because you avoid pulling the entire history.
// We can eventually increase privacy and performance (fewer calls to scan())
// by 'mining' addresses with the same prefix, allowing us to fetch the
// prefix group. Obelisk will eventually support privacy enhanced history for
// address scan by prefix.
console_result fetch_history::invoke(std::ostream &output, std::ostream &error)
{
    // Bound parameters.
    const auto &encoding = get_format_option();
    const auto &address = get_payment_address_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    callback_state state(error, output, encoding);

    // This enables json-style array formatting.
    const auto json = encoding == encoding_engine::json;

    auto on_done = [&state, json](const history::list &rows) {
        state.output(json_helper().prop_tree(rows, json));
    };

    auto on_error = [&state](const code &error) {
        state.succeeded(error);
    };

    // The v3 client API works with and normalizes either server API.
    //// client.address_fetch_history(on_error, on_done, address);
    /* client.address_fetch_history2(on_error, on_done, address); */
    client.address_fetch_history2(on_error, on_done, address);
    client.wait();

    return state.get_result();
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
