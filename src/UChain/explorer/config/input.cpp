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
#include <UChain/explorer/config/input.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/config/point.hpp>
#include <UChain/explorer/utility.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

// input is currently a private encoding in bx.
static bool decode_input(tx_input_type &input, const std::string &tuple)
{
    const auto tokens = split(tuple, BX_TX_POINT_DELIMITER);
    if (tokens.size() != 2 && tokens.size() != 3)
        return false;

    input.sequence = max_input_sequence;
    input.previous_output = point(tokens[0] + ":" + tokens[1]);

    if (tokens.size() == 3)
        deserialize(input.sequence, tokens[2], true);

    return true;
}

// input is currently a private encoding in bx.
static std::string encode_input(const tx_input_type &input)
{
    std::stringstream result;
    result << point(input.previous_output) << BX_TX_POINT_DELIMITER << input.sequence;
    return result.str();
}

input::input()
    : value_()
{
}

input::input(const std::string &tuple)
{
    std::stringstream(tuple) >> *this;
}

input::input(const tx_input_type &value)
    : value_(value)
{
}

input::input(const input &other)
    : input(other.value_)
{
}

input::input(const chain::input_point &value)
    : value_({value, {}, max_input_sequence})
{
}

input::operator const tx_input_type &() const
{
    return value_;
}

std::istream &operator>>(std::istream &stream, input &argument)
{
    std::string tuple;
    stream >> tuple;

    if (!decode_input(argument.value_, tuple))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
    }

    return stream;
}

std::ostream &operator<<(std::ostream &output, const input &argument)
{
    output << encode_input(argument.value_);
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
