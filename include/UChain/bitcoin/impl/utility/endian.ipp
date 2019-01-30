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
#ifndef UC_ENDIAN_IPP
#define UC_ENDIAN_IPP

#include <type_traits>

namespace libbitcoin
{

#define VERIFY_UNSIGNED(T) static_assert(std::is_unsigned<T>::value, \
                                         "The endian functions only work on unsigned types")

template <typename T, typename Iterator>
T from_big_endian(Iterator start, const Iterator end)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    size_t i = sizeof(T);
    while (0 < i && start != end)
        out |= static_cast<T>(*start++) << (8 * --i);

    return out;
}

template <typename T, typename Iterator>
T from_little_endian(Iterator start, const Iterator end)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    size_t i = 0;
    while (i < sizeof(T) && start != end)
        out |= static_cast<T>(*start++) << (8 * i++);

    return out;
}

template <typename T, typename Iterator>
T from_big_endian_unsafe(Iterator in)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    size_t i = sizeof(T);
    while (0 < i)
        out |= static_cast<T>(*in++) << (8 * --i);
    return out;
}

template <typename T, typename Iterator>
T from_little_endian_unsafe(Iterator in)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    size_t i = 0;
    while (i < sizeof(T))
        out |= static_cast<T>(*in++) << (8 * i++);

    return out;
}

template <typename T>
T from_big_endian_stream_unsafe(std::istream &stream)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    for (size_t i = sizeof(T); (i > 0) && stream; i--)
    {
        uint8_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof value);
        out |= static_cast<T>(value) << (8 * (i - 1));
    }

    return out;
}

template <typename T>
T from_little_endian_stream_unsafe(std::istream &stream)
{
    VERIFY_UNSIGNED(T);
    T out = 0;
    for (size_t i = 0; (i < sizeof(T)) && stream; i++)
    {
        uint8_t value = 0;
        stream.read(reinterpret_cast<char *>(&value), sizeof value);
        out |= static_cast<T>(value) << (8 * i);
    }

    return out;
}

template <typename T>
byte_array<sizeof(T)> to_big_endian(T n)
{
    VERIFY_UNSIGNED(T);
    byte_array<sizeof(T)> out;
    for (auto i = out.rbegin(); i != out.rend(); ++i)
    {
        *i = static_cast<uint8_t>(n);
        n >>= 8;
    }

    return out;
}

template <typename T>
byte_array<sizeof(T)> to_little_endian(T n)
{
    VERIFY_UNSIGNED(T);
    byte_array<sizeof(T)> out;
    for (auto i = out.begin(); i != out.end(); ++i)
    {
        *i = static_cast<uint8_t>(n);
        n >>= 8;
    }

    return out;
}

#undef VERIFY_UNSIGNED

} // namespace libbitcoin

#endif
