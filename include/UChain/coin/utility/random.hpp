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
#ifndef UC_RANDOM_HPP
#define UC_RANDOM_HPP

#include <cstdint>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/asio.hpp>
#include <UChain/coin/utility/data.hpp>

namespace libbitcoin
{

/**
 * Generate a pseudo random number within the domain.
 * @return  The 64 bit number (use % to subset domain).
 */
BC_API uint64_t pseudo_random();

/**
 * Generate a non-zero pseudo random number within the domain.
 * @return  The 64 bit number (use % to subset domain).
 */
BC_API uint64_t nonzero_pseudo_random();

/**
 * Fill a buffer with randomness using the default random engine.
 * @param[in]  chunk  The buffer to fill with randomness.
 */
BC_API void pseudo_random_fill(data_chunk &chunk);

/**
 * Convert a time duration to a value in the range [max/ratio, max].
 * @param[in]  maximum  The maximum value to return.
 * @param[in]  ratio    The determinant of the minimum duration as the inverse
 *                      portion of the maximum duration.
 * @return              The randomized duration.
 */
BC_API asio::duration pseudo_randomize(const asio::duration &maximum,
                                       uint8_t ratio = 2);

} // namespace libbitcoin

#endif
