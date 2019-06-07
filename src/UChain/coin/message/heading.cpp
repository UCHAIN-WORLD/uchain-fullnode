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
#include <UChain/coin/message/heading.hpp>

#include <boost/iostreams/stream.hpp>
#include <UChain/coin/messages.hpp>
#include <UChain/coin/message/version.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>

namespace libbitcoin
{
namespace message
{

size_t heading::maximum_size()
{
    // This assumes that the heading doesn't shrink in size.
    return serialized_size();
}

// A maximal inventory is 50,000 entries, the largest valid message.
// Inventory with 50,000 entries is 3 + 36 * 50,000 bytes (1,800,003).
// The variable integer portion is maximum 3 bytes (with a count of 50,000).
// According to protocol documentation get_blocks is limited only by the
// general maximum payload size of 0x02000000 (33,554,432). But this is an
// absurd limit for a message that should always be very small.
size_t heading::maximum_payload_size(uint32_t)
{
    static constexpr size_t vector = sizeof(uint32_t) + hash_size;
    static constexpr size_t maximum = 3u + vector * 50000u;
    static_assert(maximum <= max_size_t, "maximum_payload_size overflow");
    return maximum;
}

size_t heading::serialized_size()
{
    return sizeof(uint32_t) + command_size + sizeof(uint32_t) +
           sizeof(uint32_t);
}

heading heading::factory_from_data(const data_chunk &data)
{
    heading instance;
    instance.from_data(data);
    return instance;
}

heading heading::factory_from_data(std::istream &stream)
{
    heading instance;
    instance.from_data(stream);
    return instance;
}

heading heading::factory_from_data(reader &source)
{
    heading instance;
    instance.from_data(source);
    return instance;
}

bool heading::is_valid() const
{
    return (magic != 0) || (payload_size != 0) || (checksum != 0) || !command.empty();
}

void heading::reset()
{
    magic = 0;
    command.clear();
    command.shrink_to_fit();
    payload_size = 0;
    checksum = 0;
}

bool heading::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool heading::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool heading::from_data(reader &source)
{
    reset();
    magic = source.read_4_bytes_little_endian();
    command = source.read_fixed_string(command_size);
    payload_size = source.read_4_bytes_little_endian();
    checksum = source.read_4_bytes_little_endian();

    if (!source)
        reset();

    return source;
}

data_chunk heading::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == heading::serialized_size());
    return data;
}

void heading::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void heading::to_data(writer &sink) const
{
    sink.write_4_bytes_little_endian(magic);
    sink.write_fixed_string(command, command_size);
    sink.write_4_bytes_little_endian(payload_size);
    sink.write_4_bytes_little_endian(checksum);
}

message_type heading::type() const
{
    // TODO: convert to static map.
    if (command == address::command)
        return message_type::address;
    if (command == block_transactions::command)
        return message_type::block_transactions;
    if (command == block_msg::command)
        return message_type::block_msg;
    if (command == compact_block::command)
        return message_type::compact_block;
    if (command == filter_add::command)
        return message_type::filter_add;
    if (command == filter_clear::command)
        return message_type::filter_clear;
    if (command == filter_load::command)
        return message_type::filter_load;
    if (command == get_address::command)
        return message_type::get_address;
    if (command == get_block_transactions::command)
        return message_type::get_block_transactions;
    if (command == get_blocks::command)
        return message_type::get_blocks;
    if (command == get_data::command)
        return message_type::get_data;
    if (command == get_headers::command)
        return message_type::get_headers;
    if (command == headers::command)
        return message_type::headers;
    if (command == inventory::command)
        return message_type::inventory;
    if (command == memory_pool::command)
        return message_type::memory_pool;
    if (command == merkle_block::command)
        return message_type::merkle_block;
    if (command == not_found::command)
        return message_type::not_found;
    if (command == ping::command)
        return message_type::ping;
    if (command == pong::command)
        return message_type::pong;
    if (command == reject::command)
        return message_type::reject;
    if (command == send_compact_blocks::command)
        return message_type::send_compact_blocks;
    if (command == send_headers::command)
        return message_type::send_headers;
    if (command == transaction_message::command)
        return message_type::transaction_message;
    if (command == verack::command)
        return message_type::verack;
    if (command == version::command)
        return message_type::version;

    return message_type::unknown;
}

bool operator==(const heading &left, const heading &right)
{
    return (left.magic == right.magic) && (left.command == right.command) && (left.payload_size == right.payload_size) && (left.checksum == right.checksum);
}

bool operator!=(const heading &left, const heading &right)
{
    return !(left == right);
}

} // namespace message
} // namespace libbitcoin
