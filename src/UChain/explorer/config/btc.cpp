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
#include <UChain/explorer/config/btc.hpp>

#include <cstdint>
#include <iostream>
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

btc::btc()
    : value_(0)
{
}

btc::btc(const std::string &btc)
{
  std::stringstream(btc) >> *this;
}

btc::btc(uint64_t satoshi)
    : value_(satoshi)
{
}

btc::btc(const btc &other)
    : value_(other.value_)
{
}

btc::operator uint64_t() const
{
  return value_;
}

std::istream &operator>>(std::istream &input, btc &argument)
{
  std::string bitcoins;
  input >> bitcoins;

  if (!decode_base10(argument.value_, bitcoins, btc_decimal_places))
  {
    BOOST_THROW_EXCEPTION(invalid_option_value(bitcoins));
  }

  return input;
}

std::ostream &operator<<(std::ostream &output, const btc &argument)
{
  std::string bitcoins;
  bitcoins = encode_base10(argument.value_, btc_decimal_places);
  output << bitcoins;
  return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
