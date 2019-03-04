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
#include <UChain/explorer/config/cert_key.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <UChain/client.hpp>
#include <UChain/explorer/define.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

//constexpr uint8_t cert_key_byte_size = 32;
constexpr uint8_t cert_key_string_length = 40;

cert_key::cert_key()
    : value_()
{
}

cert_key::cert_key(const std::string &base85)
{
    std::stringstream(base85) >> *this;
}

cert_key::cert_key(const cert_key &other)
    : value_(other.value_)
{
}

// Returns empty string if not initialized.
std::string cert_key::get_base85() const
{
    std::stringstream base85;
    base85 << *this;
    return base85.str();
}

cert_key::operator const data_chunk &() const
{
    return value_;
}

cert_key::operator data_slice() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, cert_key &argument)
{
    std::string base85;
    input >> base85;

    if (base85.size() != cert_key_string_length ||
        !decode_base85(argument.value_, base85))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(base85));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const cert_key &argument)
{
    std::string decoded;

    // Z85 is unusual in that it requires four byte alignment.
    // We have already guarded construction against this, so we can ignore here.
    /* bool */ encode_base85(decoded, argument.value_);

    output << decoded;
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
