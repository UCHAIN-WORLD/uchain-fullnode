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
#include <UChain/explorer/config/encoding.hpp>

#include <exception>
#include <iostream>
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

// DRY
static auto encoding_info = "info";
static auto encoding_json = "json";
static auto encoding_xml = "xml";

encoding::encoding()
    : encoding(encoding_engine::json)
{
}

encoding::encoding(const std::string &token)
{
    std::stringstream(token) >> *this;
}

encoding::encoding(encoding_engine engine)
    : value_(engine)
{
}

encoding::encoding(const encoding &other)
    : value_(other.value_)
{
}

encoding::operator encoding_engine() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, encoding &argument)
{
    std::string text;
    input >> text;

    if (text == encoding_info)
        argument.value_ = encoding_engine::info;
    else if (text == encoding_json)
        argument.value_ = encoding_engine::json;
    else if (text == encoding_xml)
        argument.value_ = encoding_engine::xml;
    else
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(text));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const encoding &argument)
{
    std::string value;

    switch (argument.value_)
    {
    case encoding_engine::info:
        value = encoding_info;
        break;
    case encoding_engine::json:
        value = encoding_json;
        break;
    case encoding_engine::xml:
        value = encoding_xml;
        break;
    default:
        BITCOIN_ASSERT_MSG(false, "Unexpected encoding value.");
    }

    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
