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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_DATABASE_REMAINDER_IPP
#define UC_DATABASE_REMAINDER_IPP

#include <cstdint>
#include <functional>
#include <UChain/bitcoin.hpp>

namespace libbitcoin
{
namespace database
{

/// Return a hash of the key reduced to the domain of the divisor.
template <typename KeyType, typename Divisor>
Divisor remainder(const KeyType &key, const Divisor divisor)
{
    return divisor == 0 ? 0 : std::hash<KeyType>()(key) % divisor;
}

} // namespace database
} // namespace libbitcoin

#endif
