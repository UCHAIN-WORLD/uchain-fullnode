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
#include <UChain/explorer/config/address.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <UChain/coin.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace config
{

address::address(const std::string &base58)
{
    std::stringstream(base58) >> *this;
}

address::operator const std::string &() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, address &argument)
{
    std::string base58;
    input >> base58;

    bc::wallet::payment_address payment(base58);
    if (payment)
    {
        argument.value_ = base58;
        return input;
    }

    bc::wallet::stealth_address stealth(base58);
    if (stealth)
    {
        argument.value_ = base58;
        return input;
    }

    using namespace boost::program_options;
    BOOST_THROW_EXCEPTION(invalid_option_value(base58));
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
