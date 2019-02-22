/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
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
#ifndef UC_PARSE_ENCRYPTED_PREFIX_IPP
#define UC_PARSE_ENCRYPTED_PREFIX_IPP

#include <cstdint>
#include <cstddef>
#include <UChain/bitcoin/utility/data.hpp>
#include <UChain/bitcoin/wallet/encrypted_keys.hpp>

namespace libbitcoin
{
namespace wallet
{

template <size_t Size>
parse_encrypted_prefix<Size>::parse_encrypted_prefix(
    const byte_array<Size> &value)
    : prefix_(value), valid_(false)
{
}

template <size_t Size>
uint8_t parse_encrypted_prefix<Size>::context() const
{
    return prefix_.back();
}

template <size_t Size>
byte_array<Size> parse_encrypted_prefix<Size>::prefix() const
{
    return prefix_;
}

template <size_t Size>
bool parse_encrypted_prefix<Size>::valid() const
{
    return valid_;
}

template <size_t Size>
void parse_encrypted_prefix<Size>::valid(bool value)
{
    valid_ = value;
}

} // namespace wallet
} // namespace libbitcoin

#endif
