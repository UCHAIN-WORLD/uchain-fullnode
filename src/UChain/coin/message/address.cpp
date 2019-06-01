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
#include <UChain/coin/message/address.hpp>

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

const std::string address::command = "addr";
const uint32_t address::version_minimum = version::level::minimum;
const uint32_t address::version_maximum = version::level::maximum;

address address::factory_from_data(uint32_t version, const data_chunk &data)
{
    address instance;
    instance.from_data(version, data);
    return instance;
}

address address::factory_from_data(uint32_t version, std::istream &stream)
{
    address instance;
    instance.from_data(version, stream);
    return instance;
}

address address::factory_from_data(uint32_t version, reader &source)
{
    address instance;
    instance.from_data(version, source);
    return instance;
}

bool address::is_valid() const
{
    return !addresses.empty();
}

void address::reset()
{
    addresses.clear();
    addresses.shrink_to_fit();
}

bool address::from_data(uint32_t version, const data_chunk &data)
{
    data_source istream(data);
    return from_data(version, istream);
}

bool address::from_data(uint32_t version, std::istream &stream)
{
    istream_reader source(stream);
    return from_data(version, source);
}

bool address::from_data(uint32_t version, reader &source)
{
    reset();

    uint64_t count = source.read_variable_uint_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
    {
        addresses.resize(count);

        for (auto &address : addresses)
        {
            result = address.from_data(version, source, true);

            if (!result)
                break;
        }
    }

    if (!result)
        reset();

    return result;
}

data_chunk address::to_data(uint32_t version) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(version, ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version));
    return data;
}

void address::to_data(uint32_t version, std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(version, sink);
}

void address::to_data(uint32_t version, writer &sink) const
{
    sink.write_variable_uint_little_endian(addresses.size());
    for (const network_address &net_address : addresses)
        net_address.to_data(version, sink, true);
}

uint64_t address::serialized_size(uint32_t version) const
{
    return variable_uint_size(addresses.size()) +
           (addresses.size() * network_address::satoshi_fixed_size(version, true));
}

} // namespace message
} // namespace libbitcoin
