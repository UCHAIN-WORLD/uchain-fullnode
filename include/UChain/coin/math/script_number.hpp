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
#ifndef UC_SCRIPT_NUMBER_HPP
#define UC_SCRIPT_NUMBER_HPP

#include <cstddef>
#include <UChain/coin/compat.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/data.hpp>

namespace libbitcoin
{

BC_CONSTEXPR size_t max_script_number_size = 4;
BC_CONSTEXPR size_t cltv_max_script_number_size = 5;

/**
 * Numeric opcodes (OP_1ADD, etc) are restricted to operating on
 * 4-byte integers. The semantics are subtle, though: operands must be
 * in the range [-2^31 +1...2^31 -1], but results may overflow (and are
 * valid as long as they are not used in a subsequent numeric operation).
 *
 * script_number enforces those semantics by storing results as
 * an int64 and allowing out-of-range values to be returned as a vector of
 * bytes but throwing an exception if arithmetic is done or the result is
 * interpreted as an integer.
 */
class script_number
{
  public:
    BC_API explicit script_number(const int64_t value);

    // Undefined state. set_data() must be called after.
    BC_API script_number();
    BC_API bool set_data(const data_chunk &data,
                         uint8_t max_size = max_script_number_size);

    BC_API data_chunk data() const;
    BC_API int32_t int32() const;
    BC_API int64_t int64() const;

    // Arithmetic with a number.
    BC_API script_number operator+(const int64_t value) const;
    BC_API script_number operator-(const int64_t value) const;

    // Arithmetic with another script_number.
    BC_API script_number operator+(const script_number &other) const;
    BC_API script_number operator-(const script_number &other) const;

    // -script_number
    BC_API script_number operator-() const;

    // Comparison operators with a number.
    BC_API bool operator==(const int64_t value) const;
    BC_API bool operator!=(const int64_t value) const;
    BC_API bool operator<=(const int64_t value) const;
    BC_API bool operator<(const int64_t value) const;
    BC_API bool operator>=(const int64_t value) const;
    BC_API bool operator>(const int64_t value) const;

    // Comparison operators with another script_number.
    BC_API bool operator==(const script_number &other) const;
    BC_API bool operator!=(const script_number &other) const;
    BC_API bool operator<=(const script_number &other) const;
    BC_API bool operator<(const script_number &other) const;
    BC_API bool operator>=(const script_number &other) const;
    BC_API bool operator>(const script_number &other) const;

    BC_API script_number &operator+=(const int64_t value);
    BC_API script_number &operator-=(const int64_t value);
    BC_API script_number &operator+=(const script_number &other);
    BC_API script_number &operator-=(const script_number &other);

  private:
    int64_t value_;
};

} // namespace libbitcoin

#endif
