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
#include <UChain/coin/chain/header.hpp>

#include <utility>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/constants.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChainService/consensus/libdevcore/FixedHash.h>

namespace libbitcoin
{
namespace chain
{

header header::factory_from_data(const data_chunk &data,
                                 bool with_transaction_count)
{
    header instance;
    instance.from_data(data, with_transaction_count);
    return instance;
}

header header::factory_from_data(std::istream &stream,
                                 bool with_transaction_count)
{
    header instance;
    instance.from_data(stream, with_transaction_count);
    return instance;
}

header header::factory_from_data(reader &source,
                                 bool with_transaction_count)
{
    header instance;
    instance.from_data(source, with_transaction_count);
    return instance;
}

uint64_t header::satoshi_fixed_size_without_transaction_count()
{
    return 76; //148-32-8-32;
}

header::header()
{
}

header::header(const header &other)
    : header(other.version, other.previous_block_hash, other.merkle,
             other.timestamp, /*other.bits, other.nonce, other.mixhash,*/ other.number, other.transaction_count)
{
}

header::header(uint32_t version, const hash_digest &previous_block_hash,
               const hash_digest &merkle, uint32_t timestamp, /* const u256& bits,
    u64 nonce, const u256& mixhash, */
               uint32_t number, uint64_t transaction_count)
    : version(version),
      previous_block_hash(previous_block_hash),
      merkle(merkle),
      timestamp(timestamp),
      //bits(bits),
      //nonce(nonce),
      //mixhash(mixhash),
      number(number),
      transaction_count(transaction_count),
      hash_(nullptr)
{
}

header::header(header &&other)
    : header(other.version, std::forward<hash_digest>(other.previous_block_hash),
             std::forward<hash_digest>(other.merkle), other.timestamp, /*other.bits,
        other.nonce, other.mixhash,*/
                                                                       other.number,
             other.transaction_count)
{
}

header::header(uint32_t version, hash_digest &&previous_block_hash,
               hash_digest &&merkle, uint32_t timestamp, /*const u256& bits, u64 nonce, const u256& mixhash, */ uint32_t number,
               uint64_t transaction_count)
    : version(version),
      previous_block_hash(std::forward<hash_digest>(previous_block_hash)),
      merkle(std::forward<hash_digest>(merkle)),
      timestamp(timestamp),
      //bits(bits),
      //nonce(nonce),
      //mixhash(mixhash),
      number(number),
      transaction_count(transaction_count),
      hash_(nullptr)
{
}

header &header::operator=(header &&other)
{
    version = other.version;
    previous_block_hash = std::move(other.previous_block_hash);
    merkle = std::move(other.merkle);
    timestamp = other.timestamp;
    //bits = other.bits;
    //nonce = other.nonce;
    //mixhash = other.mixhash;
    number = other.number;
    transaction_count = other.transaction_count;
    return *this;
}

// TODO: eliminate header copies and then delete this.
header &header::operator=(const header &other)
{
    version = other.version;
    previous_block_hash = other.previous_block_hash;
    merkle = other.merkle;
    timestamp = other.timestamp;
    //bits = other.bits;
    //nonce = other.nonce;
    //mixhash = other.mixhash;
    number = other.number;
    transaction_count = other.transaction_count;
    return *this;
}

bool header::is_valid() const
{
    return (version != 0) ||
           (previous_block_hash != null_hash) ||
           (merkle != null_hash) ||
           (timestamp != 0) /*||
        (bits != 0) ||
        (nonce != 0)*/
        ;
}

void header::reset()
{
    version = 0;
    previous_block_hash.fill(0);
    merkle.fill(0);
    timestamp = 0;
    //bits = 0;
    //nonce = 0;

    mutex_.lock();
    hash_.reset();
    mutex_.unlock();
}

bool header::from_data(const data_chunk &data,
                       bool with_transaction_count)
{
    data_source istream(data);
    return from_data(istream, with_transaction_count);
}

bool header::from_data(std::istream &stream, bool with_transaction_count)
{
    istream_reader source(stream);
    return from_data(source, with_transaction_count);
}

bool header::from_data(reader &source, bool with_transaction_count)
{
    reset();

    version = source.read_4_bytes_little_endian();
    previous_block_hash = source.read_hash();
    merkle = source.read_hash();
    timestamp = source.read_4_bytes_little_endian();

    /*unsigned char buff[32];
    source.read_data(buff, 32);
    bits = (h256::Arith)(h256((const uint8_t*)&buff[0], h256::ConstructFromPointer));

    source.read_data(buff, 8);
    nonce = (h64::Arith)(h64((const uint8_t*)&buff[0], h64::ConstructFromPointer));

    source.read_data(buff, 32);
    mixhash = (h256::Arith)(h256((const uint8_t*)&buff[0], h256::ConstructFromPointer));*/

    number = source.read_4_bytes_little_endian();

    transaction_count = 0;
    if (with_transaction_count)
        transaction_count = source.read_variable_uint_little_endian();

    const auto result = static_cast<bool>(source);

    if (!result)
        reset();

    return result;
}

data_chunk header::to_data(bool with_transaction_count) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream, with_transaction_count);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(with_transaction_count));
    return data;
}

void header::to_data(std::ostream &stream,
                     bool with_transaction_count) const
{
    ostream_writer sink(stream);
    to_data(sink, with_transaction_count);
}

void header::to_data(writer &sink, bool with_transaction_count) const
{
    sink.write_4_bytes_little_endian(version);
    sink.write_hash(previous_block_hash);
    sink.write_hash(merkle);
    sink.write_4_bytes_little_endian(timestamp);

    //sink.write_data((unsigned char*)h256(bits).data(), 32);
    //sink.write_data((unsigned char*)h64(nonce).data(), 8);
    //sink.write_data((unsigned char*)h256(mixhash).data(), 32);

    sink.write_4_bytes_little_endian(number);

    if (with_transaction_count)
        sink.write_variable_uint_little_endian(transaction_count);
}

uint64_t header::serialized_size(bool with_transaction_count) const
{
    auto size = satoshi_fixed_size_without_transaction_count();

    if (with_transaction_count)
        size += variable_uint_size(transaction_count);

    return size;
}

hash_digest header::hash() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (!hash_)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        hash_.reset(new hash_digest(bitcoin_hash(to_data(false))));
        mutex_.unlock_and_lock_upgrade();
        //---------------------------------------------------------------------
    }

    hash_digest hash = *hash_;
    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return hash;
}

bool operator==(const header &left, const header &right)
{
    return (left.version == right.version) && (left.previous_block_hash == right.previous_block_hash) && (left.merkle == right.merkle) && (left.timestamp == right.timestamp)
           /*&& (left.bits == right.bits)
        && (left.nonce == right.nonce)*/
           && (left.transaction_count == right.transaction_count);
}

bool operator!=(const header &left, const header &right)
{
    return !(left == right);
}

} // namespace chain
} // namespace libbitcoin
