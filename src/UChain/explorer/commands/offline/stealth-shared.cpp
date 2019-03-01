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
#include <UChain/explorer/commands/stealth-shared.hpp>

#include <iostream>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>

// This is nearly the same as ec-multiply + sha256.
// Pass either (ephem_secret, scan_pubkey) or (scan_secret, ephem_pubkey).
namespace libbitcoin
{
namespace explorer
{
namespace commands
{

console_result stealth_shared::invoke(std::ostream &output,
                                      std::ostream &error)
{
    // Bound parameters.
    const auto &secret = get_secret_argument();
    const auto &pubkey = get_pubkey_argument();

    ec_compressed product(pubkey);
    if (!bc::ec_multiply(product, secret))
    {
        error << BX_STEALTH_SHARED_OUT_OF_RANGE << std::flush;
        return console_result::failure;
    }

    const auto hash = sha256_hash(product);

    output << config::ec_private(hash) << std::flush;
    return console_result::okay;
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
