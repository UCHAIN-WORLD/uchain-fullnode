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
#include <UChain/coin/message/block_txs.hpp>

#include <boost/iostreams/stream.hpp>
#include <UChain/coin/message/version.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>

namespace libbitcoin
{
namespace message
{

const std::string block_txs::command = "blocktxn";
const uint32_t block_txs::version_minimum = version::level::bip152;
const uint32_t block_txs::version_maximum = version::level::bip152;

block_txs block_txs::factory_from_data(
    const uint32_t version, const data_chunk &data)
{
    block_txs instance;
    instance.from_data(version, data);
    return instance;
}

block_txs block_txs::factory_from_data(
    const uint32_t version, std::istream &stream)
{
    block_txs instance;
    instance.from_data(version, stream);
    return instance;
}

block_txs block_txs::factory_from_data(
    const uint32_t version, reader &source)
{
    block_txs instance;
    instance.from_data(version, source);
    return instance;
}

bool block_txs::is_valid() const
{
    return (block_hash != null_hash);
}

void block_txs::reset()
{
    block_hash = null_hash;
    transactions.clear();
    transactions.shrink_to_fit();
}

bool block_txs::from_data(uint32_t version,
                                   const data_chunk &data)
{
    data_source istream(data);
    return from_data(version, istream);
}

bool block_txs::from_data(uint32_t version,
                                   std::istream &stream)
{
    istream_reader source(stream);
    return from_data(version, source);
}

bool block_txs::from_data(uint32_t version, reader &source)
{
    reset();
    auto result = !(version < block_txs::version_minimum);
    block_hash = source.read_hash();
    result &= static_cast<bool>(source);
    const auto count = source.read_variable_uint_little_endian();
    result &= static_cast<bool>(source);

    if (result)
    {
        transactions.resize(count);

        for (auto &transaction : transactions)
        {
            result = transaction.from_data(source);

            if (!result)
                break;
        }
    }

    if (!result)
        reset();

    return result;
}

data_chunk block_txs::to_data(uint32_t version) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(version, ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version));
    return data;
}

void block_txs::to_data(uint32_t version,
                                 std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(version, sink);
}

void block_txs::to_data(uint32_t version, writer &sink) const
{
    sink.write_hash(block_hash);
    sink.write_variable_uint_little_endian(transactions.size());

    for (const auto &element : transactions)
        element.to_data(sink);
}

uint64_t block_txs::serialized_size(uint32_t version) const
{
    uint64_t size = hash_size + variable_uint_size(transactions.size());

    for (const auto &element : transactions)
        size += element.serialized_size();

    return size;
}

} // namespace message
} // namespace libbitcoin
