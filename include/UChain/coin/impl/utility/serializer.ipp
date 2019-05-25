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
#ifndef UC_SERIALIZER_IPP
#define UC_SERIALIZER_IPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <boost/asio/streambuf.hpp>
#include <UChain/coin/error.hpp>
#include <UChain/coin/utility/assert.hpp>
#include <UChain/coin/utility/endian.hpp>

namespace libbitcoin
{

template <typename Iterator>
serializer<Iterator>::serializer(const Iterator begin)
    : iterator_(begin)
{
}

template <typename Iterator>
serializer<Iterator>::operator bool() const
{
    return true;
}

template <typename Iterator>
bool serializer<Iterator>::operator!() const
{
    return false;
}

template <typename Iterator>
void serializer<Iterator>::write_byte(uint8_t value)
{
    *iterator_ = value;
    ++iterator_;
}

template <typename Iterator>
void serializer<Iterator>::write_data(const data_chunk &data)
{
    write_data<const data_chunk>(data);
}

template <typename Iterator>
void serializer<Iterator>::write_data(const uint8_t *data, size_t size)
{
    iterator_ = std::copy(data, (data + size), iterator_);
}

template <typename Iterator>
void serializer<Iterator>::write_error_code(const code &ec)
{
    const auto value = ec.value();
    BITCOIN_ASSERT(value >= 0 && static_cast<uint32_t>(value) <= max_uint32);
    write_4_bytes_little_endian(static_cast<uint32_t>(value));
}

template <typename Iterator>
void serializer<Iterator>::write_2_bytes_little_endian(uint16_t value)
{
    write_little_endian(value);
}

template <typename Iterator>
void serializer<Iterator>::write_4_bytes_little_endian(uint32_t value)
{
    write_little_endian(value);
}

template <typename Iterator>
void serializer<Iterator>::write_8_bytes_little_endian(uint64_t value)
{
    write_little_endian(value);
}

template <typename Iterator>
void serializer<Iterator>::write_2_bytes_big_endian(uint16_t value)
{
    write_big_endian(value);
}

template <typename Iterator>
void serializer<Iterator>::write_4_bytes_big_endian(uint32_t value)
{
    write_big_endian(value);
}

template <typename Iterator>
void serializer<Iterator>::write_8_bytes_big_endian(uint64_t value)
{
    write_big_endian(value);
}

template <typename Iterator>
template <typename T>
void serializer<Iterator>::write_big_endian(T n)
{
    return write_data(to_big_endian(n));
}

template <typename Iterator>
template <typename T>
void serializer<Iterator>::write_little_endian(T n)
{
    return write_data(to_little_endian(n));
}

template <typename Iterator>
void serializer<Iterator>::write_variable_uint_little_endian(uint64_t value)
{
    if (value < 0xfd)
    {
        write_byte((uint8_t)value);
    }
    else if (value <= 0xffff)
    {
        write_byte(0xfd);
        write_2_bytes_little_endian((uint16_t)value);
    }
    else if (value <= 0xffffffff)
    {
        write_byte(0xfe);
        write_4_bytes_little_endian((uint32_t)value);
    }
    else
    {
        write_byte(0xff);
        write_8_bytes_little_endian(value);
    }
}

template <typename Iterator>
void serializer<Iterator>::write_variable_uint_big_endian(uint64_t value)
{
    if (value < 0xfd)
    {
        write_byte((uint8_t)value);
    }
    else if (value <= 0xffff)
    {
        write_byte(0xfd);
        write_2_bytes_big_endian((uint16_t)value);
    }
    else if (value <= 0xffffffff)
    {
        write_byte(0xfe);
        write_4_bytes_big_endian((uint32_t)value);
    }
    else
    {
        write_byte(0xff);
        write_8_bytes_big_endian(value);
    }
}

template <typename Iterator>
template <typename T>
void serializer<Iterator>::write_data(const T &data)
{
    iterator_ = std::copy(data.begin(), data.end(), iterator_);
}

template <typename Iterator>
void serializer<Iterator>::write_hash(const hash_digest &hash)
{
    write_data(hash);
}

template <typename Iterator>
void serializer<Iterator>::write_short_hash(const short_hash &hash)
{
    write_data(hash);
}

template <typename Iterator>
void serializer<Iterator>::write_mini_hash(
    const mini_hash &hash)
{
    write_data(hash);
}

template <typename Iterator>
void serializer<Iterator>::write_fixed_string(const std::string &value,
                                              size_t size)
{
    const auto min_size = std::min(size, value.size());
    data_chunk raw_string(size, 0);

    std::copy_n(value.begin(), min_size, raw_string.begin());
    write_data(raw_string);
}

template <typename Iterator>
void serializer<Iterator>::write_string(const std::string &value)
{
    write_variable_uint_little_endian(value.size());
    write_data(value);
}

/**
 * Returns underlying iterator.
 */
template <typename Iterator>
Iterator serializer<Iterator>::iterator()
{
    return iterator_;
}

/**
 * Useful if you want to serialize some data using another
 * routine and then continue with this serializer.
 */
template <typename Iterator>
void serializer<Iterator>::set_iterator(Iterator iterator)
{
    iterator_ = iterator;
}

template <typename Iterator>
template <typename T>
void serializer<Iterator>::write_data_reverse(const T &data)
{
    iterator_ = std::reverse_copy(data.begin(), data.end(), iterator_);
}

template <typename Iterator>
serializer<Iterator> make_serializer(Iterator begin)
{
    return serializer<Iterator>(begin);
}

} // namespace libbitcoin

#endif
