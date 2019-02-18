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
#include <UChain/bitcoin/chain/point.hpp>

#include <cstdint>
#include <sstream>
#include <tuple>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/constants.hpp>
#include <UChain/bitcoin/formats/base_16.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>
#include <UChain/bitcoin/utility/serializer.hpp>

namespace libbitcoin
{
namespace chain
{

point point::factory_from_data(const data_chunk &data)
{
    point instance;
    instance.from_data(data);
    return instance;
}

point point::factory_from_data(std::istream &stream)
{
    point instance;
    instance.from_data(stream);
    return instance;
}

point point::factory_from_data(reader &source)
{
    point instance;
    instance.from_data(source);
    return instance;
}

bool point::is_valid() const
{
    return (index != 0) || (hash != null_hash);
}

void point::reset()
{
    hash.fill(0);
    index = 0;
}

bool point::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool point::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool point::from_data(reader &source)
{
    reset();

    hash = source.read_hash();
    index = source.read_4_bytes_little_endian();
    const auto result = static_cast<bool>(source);

    if (!result)
        reset();

    return result;
}

data_chunk point::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void point::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void point::to_data(writer &sink) const
{
    sink.write_hash(hash);
    sink.write_4_bytes_little_endian(index);
}

uint64_t point::serialized_size() const
{
    return point::satoshi_fixed_size();
}

uint64_t point::satoshi_fixed_size()
{
    return hash_size + sizeof(uint32_t);
}

point_iterator point::begin() const
{
    return point_iterator(*this);
}

point_iterator point::end() const
{
    return point_iterator(*this, true);
}

std::string point::to_string() const
{
    std::ostringstream value;
    value << "\thash = " << encode_hash(hash) << "\n\tindex = " << index;
    return value.str();
}

bool point::is_null() const
{
    return index == max_uint32 && hash == null_hash;
}
// Changed in v3.0 and again in v3.1 (3.0 was unmasked, lots of collisions).
// This is used with output_point identification within a set of history rows
// of the same address. Collision will result in miscorrelation of points by
// client callers. This is stored in database. This is NOT a bitcoin checksum.
uint64_t point::checksum() const
{
    // Reserve 49 bits for the tx hash and 15 bits (32768) for the input index.
    static constexpr uint64_t mask = 0xffffffffffff8000;

    // Use an offset to the middle of the hash to avoid coincidental mining
    // of values into the front or back of tx hash (not a security feature).
    // Use most possible bits of tx hash to make intentional collision hard.
    const auto tx = from_little_endian_unsafe<uint64_t>(hash.begin() + 12);
    const auto tx_index = static_cast<uint64_t>(index);

    const auto tx_upper_49_bits = tx & mask;
    const auto index_lower_15_bits = tx_index & ~mask;
    return tx_upper_49_bits | index_lower_15_bits;
}
#if 0 // old version checksum method
// This is used with output_point identification within a set of history rows
// of the same address. Collision will result in miscorrelation of points by
// client callers. This is NOT a bitcoin checksum.
uint64_t point::checksum() const
{
    static constexpr uint64_t divisor = uint64_t{ 1 } << 63;
    static_assert(divisor == 9223372036854775808ull, "Wrong divisor value.");

    // Write index onto a copy of the outpoint hash.
    auto copy = hash;
    auto serial = make_serializer(copy.begin());
    serial.write_4_bytes_little_endian(index);
    const auto hash_value = from_little_endian_unsafe<uint64_t>(copy.begin());

    // x mod 2**n == x & (2**n - 1)
    return hash_value & (divisor - 1);

    // Above usually provides only 32 bits of entropy, so below is preferred.
    // But this is stored in the database. Change requires server API change.
    // return std::hash<point>()(*this);
}
#endif
bool operator==(const point &left, const point &right)
{
    return left.hash == right.hash && left.index == right.index;
}

bool operator!=(const point &left, const point &right)
{
    return !(left == right);
}

bool operator<(const point &left, const point &right)
{
    typedef std::tuple<hash_digest, uint32_t> tupe_cmp;
    return tupe_cmp(left.hash, left.index) < tupe_cmp(right.hash, right.index);
}

} // namespace chain
} // namespace libbitcoin
