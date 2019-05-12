/**
 * Copyright (c) 2018-2020 UChain developers 
 *
 * This file is part of uc-node.
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
#include <UChainService/txs/uid/uid.hpp>
#include <UChainService/txs/variant.hpp>
#include <UChainService/txs/uid/uid_detail.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin
{
namespace chain
{

uid::uid()
{
    reset();
}
uid::uid(uint32_t status, const uid_detail &detail) : status(status), data(detail)
{
}

uid uid::factory_from_data(const data_chunk &data)
{
    uid instance;
    instance.from_data(data);
    return instance;
}

uid uid::factory_from_data(std::istream &stream)
{
    uid instance;
    instance.from_data(stream);
    return instance;
}

uid uid::factory_from_data(reader &source)
{
    uid instance;
    instance.from_data(source);
    return instance;
}

void uid::reset()
{
    status = 0; //uid_status::uid_none;
    data.reset();
}

bool uid::is_valid() const
{
    return data.is_valid();
}

bool uid::is_valid_type() const
{
    return ((UID_DETAIL_TYPE == status) || (UID_TRANSFERABLE_TYPE == status));
}

bool uid::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool uid::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool uid::from_data(reader &source)
{
    reset();

    status = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type())
    {
        result = data.from_data(source);
    }
    else
    {
        result = false;
        reset();
    }

    return result;
}

data_chunk uid::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void uid::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void uid::to_data(writer &sink) const
{
    sink.write_4_bytes_little_endian(status);
    data.to_data(sink);
}

uint64_t uid::serialized_size() const
{
    return 4 + data.serialized_size();
}

std::string uid::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << status << "\n";
    ss << data.to_string();
    return ss.str();
}

uint32_t uid::get_status() const
{
    return status;
}
void uid::set_status(uint32_t status)
{
    this->status = status;
}
void uid::set_data(const uid_detail &detail)
{
    this->data = detail;
}

const uid_detail &uid::get_data() const
{
    return this->data;
}

} // namespace chain
} // namespace libbitcoin
