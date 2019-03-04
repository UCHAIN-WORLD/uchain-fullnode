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
#include <UChain/explorer/config/ec_private.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

// ec_secret base16 format is private to bx.
static bool decode_secret(ec_secret &secret, const std::string &encoded)
{
    return decode_base16(secret, encoded) && verify(secret);
}

ec_private::ec_private(const std::string &hexcode)
{
    std::stringstream(hexcode) >> *this;
}

ec_private::ec_private(const ec_secret &secret)
    : value_(secret)
{
}

ec_private::operator const ec_secret &() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, ec_private &argument)
{
    std::string hexcode;
    input >> hexcode;

    if (!decode_secret(argument.value_, hexcode))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(hexcode));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const ec_private &argument)
{
    output << encode_base16(argument.value_);
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
