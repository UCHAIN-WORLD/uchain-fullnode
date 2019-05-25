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
#include <UChain/explorer/config/script.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/utility.hpp>

using namespace po;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

script::script()
    : value_()
{
}

script::script(const std::string &mnemonic)
{
    std::stringstream(mnemonic) >> *this;
}

script::script(const chain::script &value)
    : value_(value)
{
}

script::script(const data_chunk &value)
{
    value_.from_data(value, false,
                     chain::script::parse_mode::raw_data_fallback);
}

script::script(const std::vector<std::string> &tokens)
{
    const auto mnemonic = join(tokens);
    std::stringstream(mnemonic) >> *this;
}

script::script(const script &other)
    : script(other.value_)
{
}

const data_chunk script::to_data() const
{
    return value_.to_data(false);
}

const std::string script::to_string() const
{
    static constexpr auto flags = chain::script_context::all_enabled;
    return value_.to_string(flags);
}

script::operator const chain::script &() const
{
    return value_;
}

std::istream &operator>>(std::istream &input, script &argument)
{
    std::istreambuf_iterator<char> end;
    std::string mnemonic(std::istreambuf_iterator<char>(input), end);
    boost::trim(mnemonic);

    // Test for invalid result sentinel.
    if (!argument.value_.from_string(mnemonic) && mnemonic.length() > 0)
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(mnemonic));
    }

    return input;
}

std::ostream &operator<<(std::ostream &output, const script &argument)
{
    static constexpr auto flags = chain::script_context::all_enabled;
    output << argument.value_.to_string(flags);
    return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
