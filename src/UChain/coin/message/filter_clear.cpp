/*
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
#include <UChain/coin/message/filter_clear.hpp>

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

const std::string filter_clear::command = "filterclear";
const uint32_t filter_clear::version_minimum = version::level::bip37;
const uint32_t filter_clear::version_maximum = version::level::maximum;

filter_clear filter_clear::factory_from_data(uint32_t version,
                                             const data_chunk &data)
{
    filter_clear instance;
    instance.from_data(version, data);
    return instance;
}

filter_clear filter_clear::factory_from_data(uint32_t version,
                                             std::istream &stream)
{
    filter_clear instance;
    instance.from_data(version, stream);
    return instance;
}

filter_clear filter_clear::factory_from_data(uint32_t version,
                                             reader &source)
{
    filter_clear instance;
    instance.from_data(version, source);
    return instance;
}

filter_clear::filter_clear()
{
    reset();
}

bool filter_clear::is_valid() const
{
    return !insufficient_version_;
}

void filter_clear::reset()
{
    insufficient_version_ = false;
}

bool filter_clear::from_data(uint32_t version, const data_chunk &data)
{
    boost::iostreams::stream<byte_source<data_chunk>> istream(data);
    return from_data(version, istream);
}

bool filter_clear::from_data(uint32_t version, std::istream &stream)
{
    istream_reader source(stream);
    return from_data(version, source);
}

bool filter_clear::from_data(uint32_t version, reader &source)
{
    reset();
    insufficient_version_ = (version < filter_clear::version_minimum);
    return !insufficient_version_;
}

data_chunk filter_clear::to_data(uint32_t version) const
{
    data_chunk data;
    boost::iostreams::stream<byte_sink<data_chunk>> ostream(data);
    to_data(version, ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version));
    return data;
}

void filter_clear::to_data(uint32_t version, std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(version, sink);
}

void filter_clear::to_data(uint32_t version, writer &sink) const
{
}

uint64_t filter_clear::serialized_size(uint32_t version) const
{
    return filter_clear::satoshi_fixed_size(version);
}

uint64_t filter_clear::satoshi_fixed_size(uint32_t version)
{
    return 0;
}

} // namespace message
} // namespace libbitcoin
