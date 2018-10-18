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
#include <UChainService/txs/ucn/ucn.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

ucn::ucn()
{
    value = 0;
}
ucn::ucn(uint64_t value):
    value(value)
{

}

ucn ucn::factory_from_data(const data_chunk& data)
{
    ucn instance;
    instance.from_data(data);
    return instance;
}

ucn ucn::factory_from_data(std::istream& stream)
{
    ucn instance;
    instance.from_data(stream);
    return instance;
}

ucn ucn::factory_from_data(reader& source)
{
    ucn instance;
    instance.from_data(source);
    return instance;
}

void ucn::reset()
{
    value= 0;
}
bool ucn::is_valid() const
{
    return true;
}

bool ucn::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool ucn::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool ucn::from_data(reader& source)
{
    /*
    reset();
    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    return result;
    */
    return true;
}

data_chunk ucn::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void ucn::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void ucn::to_data(writer& sink) const
{
    //sink.write_8_bytes_little_endian(value); // not use ucn now
}

uint64_t ucn::serialized_size() const
{
    //uint64_t size = 8;
    //return size;
    return 0; // not insert ept into transaction
}

std::string ucn::to_string() const
{
    std::ostringstream ss;
    ss << "\t value = " << value << "\n";

    return ss.str();
}
uint64_t ucn::get_value() const
{
    return value;
}

void ucn::set_value(uint64_t value)
{
    this->value = value;
}

} // namspace chain
} // namspace libbitcoin
