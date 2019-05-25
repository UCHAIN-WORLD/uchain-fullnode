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
#ifndef UC_HASH_IPP
#define UC_HASH_IPP

#include <algorithm>
#include <cstddef>
#include <UChain/coin/utility/data.hpp>

namespace libbitcoin
{

template <size_t Size>
byte_array<Size> scrypt(data_slice data, data_slice salt, uint64_t N,
                        uint32_t p, uint32_t r)
{
    const auto out = scrypt(data, salt, N, r, p, Size);
    return to_array<Size>({out});
}

} // namespace libbitcoin

#endif
