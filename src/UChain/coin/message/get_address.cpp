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
#include <UChain/coin/message/get_address.hpp>

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

const std::string get_address::command = "getaddr";
const uint32_t get_address::version_minimum = version::level::minimum;
const uint32_t get_address::version_maximum = version::level::maximum;

get_address get_address::factory_from_data(uint32_t version,
                                           const data_chunk &data)
{
    get_address instance;
    instance.from_data(version, data);
    return instance;
}

get_address get_address::factory_from_data(uint32_t version,
                                           std::istream &stream)
{
    get_address instance;
    instance.from_data(version, stream);
    return instance;
}

get_address get_address::factory_from_data(uint32_t version,
                                           reader &source)
{
    get_address instance;
    instance.from_data(version, source);
    return instance;
}

bool get_address::is_valid() const
{
    return true;
}

void get_address::reset()
{
}

bool get_address::from_data(uint32_t version, const data_chunk &data)
{
    data_source istream(data);
    return from_data(version, istream);
}

bool get_address::from_data(uint32_t version, std::istream &stream)
{
    istream_reader source(stream);
    return from_data(version, source);
}

bool get_address::from_data(uint32_t version, reader &source)
{
    reset();
    return source;
}

data_chunk get_address::to_data(uint32_t version) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(version, ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version));
    return data;
}

void get_address::to_data(uint32_t version, std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(version, sink);
}

void get_address::to_data(uint32_t version, writer &sink) const
{
}

uint64_t get_address::serialized_size(uint32_t version) const
{
    return get_address::satoshi_fixed_size(version);
}

uint64_t get_address::satoshi_fixed_size(uint32_t version)
{
    return 0;
}

} // namespace message
} // namespace libbitcoin
