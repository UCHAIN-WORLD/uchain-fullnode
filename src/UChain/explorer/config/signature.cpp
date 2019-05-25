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
#include <UChain/explorer/config/signature.hpp>

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <boost/program_options.hpp>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

// message_signature format is currently private to bx.
static bool decode_signature(bc::wallet::message_signature &signature,
                             const std::string &encoded)
{
    // There is no bc::decode_base64 array-based override.
    data_chunk decoded;
    if (!decode_base64(decoded, encoded) ||
        (decoded.size() != bc::wallet::message_signature_size))
        return false;

    std::copy(decoded.begin(), decoded.end(), signature.begin());
    return true;
}

static std::string encode_signature(const bc::wallet::message_signature &signature)
{
    return encode_base64(signature);
}

signature::signature()
    : value_()
{
}

signature::signature(const std::string &hexcode)
{
    std::stringstream(hexcode) >> *this;
}

signature::signature(const bc::wallet::message_signature &value)
    : value_(value)
{
}

signature::signature(const signature &other)
    : signature(other.value_)
{
}

signature::operator const bc::wallet::message_signature &() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, signature &argument)
{
    std::string hexcode;
    input >> hexcode;

    if (!decode_signature(argument.value_, hexcode))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(hexcode));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const signature &argument)
{
    output << encode_signature(argument.value_);
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
