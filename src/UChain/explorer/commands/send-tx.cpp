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

#include <UChain/explorer/commands/send-tx.hpp>

#include <iostream>
#include <boost/format.hpp>
#include <UChain/client.hpp>
#include <UChain/explorer/callback_state.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/utility.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::client;

console_result send_tx::invoke(std::ostream &output, std::ostream &error)
{
    // Bound parameters.
    const auto &transaction = get_transaction_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    callback_state state(error, output);

    auto on_done = [&state]() {
        state.output(std::string(BX_SEND_TX_OUTPUT));
    };

    auto on_error = [&state](const code &error) {
        state.succeeded(error);
    };

    client.protocol_broadcast_transaction(on_error, on_done, transaction);
    client.wait();

    return state.get_result();
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
