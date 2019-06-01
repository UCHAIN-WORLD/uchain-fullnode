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
#ifndef UC_PARSE_ENCRYPTED_PREFIX_HPP
#define UC_PARSE_ENCRYPTED_PREFIX_HPP

#include <cstdint>
#include <cstddef>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/wallet/encrypted_keys.hpp>

// BIP38
// Alt-chain implementers should exploit the address hash for [identification].
// Since each operation in this proposal involves hashing a text representation
// of a coin address which (for Bitcoin) includes the leading '1', an alt-chain
// can easily be denoted simply by using the alt-chain's preferred format for
// representing an address.... Alt-chain implementers may also change the prefix
// such that encrypted addresses do not start with "6P".

namespace libbitcoin {
namespace wallet {

template<size_t Size>
class parse_encrypted_prefix
{
public:
    bool valid() const;

    static constexpr uint8_t prefix_size = Size;

protected:
    explicit parse_encrypted_prefix(const byte_array<Size>& value);

    uint8_t context() const;
    byte_array<Size> prefix() const;
    void valid(bool value);

    static constexpr uint8_t magic_size = Size - 1;

private:
    bool verify_magic() const;

    const byte_array<Size> prefix_;
    bool valid_;
};

} // namespace wallet
} // namespace libbitcoin

#include "parse_encrypted_prefix.ipp"

#endif
