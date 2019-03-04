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
#include <UChain/explorer/config/endorsement.hpp>

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
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

// endorsement format is currently private to bx.
static bool decode_endorsement(bc::endorsement &endorsement,
                               const std::string &encoded)
{
    bc::endorsement decoded;
    if (!decode_base16(decoded, encoded) ||
        (decoded.size() > max_endorsement_size))
        return false;

    endorsement = decoded;
    return true;
}

static std::string encode_endorsement(data_slice signature)
{
    return encode_base16(signature);
}

endorsement::endorsement()
    : value_()
{
}

endorsement::endorsement(const std::string &hexcode)
{
    std::stringstream(hexcode) >> *this;
}

endorsement::endorsement(const data_chunk &value)
    : value_(value)
{
}

endorsement::endorsement(const endorsement &other)
    : endorsement(other.value_)
{
}

endorsement::operator const data_chunk &() const
{
    return value_;
}

endorsement::operator data_slice() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, endorsement &argument)
{
    std::string hexcode;
    input >> hexcode;

    if (!decode_endorsement(argument.value_, hexcode))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(hexcode));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const endorsement &argument)
{
    output << encode_endorsement(argument.value_);
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
