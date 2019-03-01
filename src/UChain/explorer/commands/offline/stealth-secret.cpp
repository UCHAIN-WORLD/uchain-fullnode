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
#include <UChain/explorer/commands/stealth-secret.hpp>

#include <iostream>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/config/ec_private.hpp>

// This is nearly the same as ec-add-secrets.
namespace libbitcoin
{
namespace explorer
{
namespace commands
{

console_result stealth_secret::invoke(std::ostream &output,
                                      std::ostream &error)
{
    // Bound parameters.
    const auto &scan_secret = get_spend_secret_argument();
    const auto &shared_secret = get_shared_secret_argument();

    ec_secret sum(scan_secret);
    if (!bc::ec_add(sum, shared_secret))
    {
        error << BX_STEALTH_SECRET_OUT_OF_RANGE << std::flush;
        return console_result::failure;
    }

    output << config::ec_private(sum) << std::flush;
    return console_result::okay;
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
