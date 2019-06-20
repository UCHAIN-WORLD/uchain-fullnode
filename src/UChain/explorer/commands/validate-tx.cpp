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

#include <UChain/explorer/commands/validate-tx.hpp>

#include <iostream>
#include <UChain/client.hpp>
#include <UChain/explorer/callback_state.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/utility.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::client;
using namespace bc::explorer::config;

console_result validate_tx::invoke(std::ostream &output,
                                   std::ostream &error)
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

    auto on_done = [&state](const chain::point::indexes &indexes) {
        if (indexes.empty())
        {
            state.output(std::string(BX_VALIDATE_TX_VALID));
            return;
        }

        const auto unconfirmed = join(numbers_to_strings(indexes), ", ");
        state.output(format(BX_VALIDATE_TX_UNCONFIRMED_INPUTS) % unconfirmed);
    };

    auto on_error = [&state](const code &error) {
        // BX_VALIDATE_TX_INVALID_INPUT is not currently utilized.
        // The client suppresses an index list which may have 0 or one element.
        // The list contains the index of the input which caused the failure.
        state.succeeded(error);
    };

    client.tx_pool_validate(on_error, on_done, transaction);
    client.wait();

    return state.get_result();
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
