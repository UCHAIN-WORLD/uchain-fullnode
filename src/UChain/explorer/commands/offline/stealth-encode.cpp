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
#include <UChain/explorer/commands/stealth-encode.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::wallet;

console_result stealth_encode::invoke(std::ostream &output,
                                      std::ostream &error)
{
    // Bound parameters.
    const auto version = get_version_option();
    const auto &filter = get_filter_option();
    const auto &scan_pubkey = get_scan_pubkey_argument();
    const auto &spend_pubkeys = get_spend_pubkeys_argument();
    const auto &signatures = get_signatures_option();

    const size_t maximum = std::max(size_t(1), spend_pubkeys.size());

    if (signatures > maximum)
    {
        error << BX_STEALTH_ENCODE_SIGNATURES_OVERFLOW << std::flush;
        return console_result::failure;
    }

    // TODO: finish stealth multisig implemetation.
    // Issue a warning but don't prevent experimentation.
    if (spend_pubkeys.size() > 1u)
        error << BX_STEALTH_ENCODE_MULTISIG_NOT_SUPPORTED << std::flush;

    if (filter.size() > stealth_address::max_filter_bits)
    {
        error << BX_STEALTH_ENCODE_FILTER_TOO_LONG << std::flush;
        return console_result::failure;
    }

    const auto spend_points = cast<ec_public, ec_compressed>(spend_pubkeys);
    const stealth_address address(filter, scan_pubkey, spend_points,
                                  signatures, version);

    output << address << std::flush;
    return console_result::okay;
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
