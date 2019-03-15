/**
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

#include <UChainService/api/command/command_assistant.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

// ---------------------------------------------------------------------------
std::string ec_to_xxx_impl(const std::string &cmd, const std::string &fromkey)
{
    if (cmd == "ec-to-public")
    {

        explorer::config::ec_private secret{fromkey};
        ec_compressed point;
        bc::secret_to_public(point, secret);
        bc::wallet::ec_public ec_pubkey(point, true); // compressed for UC always

        return ec_pubkey.encoded();
    }

    if (cmd == "ec-to-address")
    {

        bc::wallet::ec_public point{fromkey};
        bc::wallet::payment_address pay_addr(point, bc::wallet::payment_address::mainnet_p2kh);

        return pay_addr.encoded();
    }

    return "";
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
