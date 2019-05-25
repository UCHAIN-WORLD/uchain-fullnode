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
#include <UChain/explorer/config/wrapper.hpp>

#include <iostream>
#include <boost/program_options.hpp>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/utility.hpp>

using namespace po;
using namespace bc::config;

namespace libbitcoin
{
namespace explorer
{
namespace config
{

wrapper::wrapper()
    : value_()
{
}

wrapper::wrapper(const std::string &wrapped)
{
  std::stringstream(wrapped) >> *this;
}

wrapper::wrapper(const data_chunk &wrapped)
    : wrapper(encode_base16(wrapped))
{
}

wrapper::wrapper(const bc::wallet::wrapped_data &wrapped)
    : value_(wrapped)
{
}

wrapper::wrapper(const bc::wallet::payment_address &address)
    : wrapper(encode_base16(address.to_payment()))
{
}

wrapper::wrapper(uint8_t version, const data_chunk &payload)
    : wrapper(bc::wallet::wrapped_data{version, payload, 0})
{
}

wrapper::wrapper(const wrapper &other)
    : value_(other.value_)
{
}

const data_chunk wrapper::to_data() const
{
  return wrap(value_);
}

wrapper::operator const bc::wallet::wrapped_data &() const
{
  return value_;
}

std::istream &operator>>(std::istream &input, wrapper &argument)
{
  std::string hexcode;
  input >> hexcode;

  // The checksum is validated here.
  if (!unwrap(argument.value_, base16(hexcode)))
  {
    BOOST_THROW_EXCEPTION(invalid_option_value(hexcode));
  }

  return input;
}

std::ostream &operator<<(std::ostream &output, const wrapper &argument)
{
  // The checksum is calculated here (value_ checksum is ignored).
  const auto bytes = wrap(argument.value_);
  output << base16(bytes);
  return output;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
