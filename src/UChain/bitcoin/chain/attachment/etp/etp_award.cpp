/**
 * Copyright (c) 2011-2018 UChain developers (see AUTHORS)
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
#include <UChain/bitcoin/chain/attachment/ucn/ucn_award.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

ucn_award::ucn_award()
{
    height = 0;
}
ucn_award::ucn_award(uint64_t height):
    height(height)
{

}

ucn_award ucn_award::factory_from_data(const data_chunk& data)
{
    ucn_award instance;
    instance.from_data(data);
    return instance;
}

ucn_award ucn_award::factory_from_data(std::istream& stream)
{
    ucn_award instance;
    instance.from_data(stream);
    return instance;
}

ucn_award ucn_award::factory_from_data(reader& source)
{
    ucn_award instance;
    instance.from_data(source);
    return instance;
}

void ucn_award::reset()
{
    height= 0;
}
bool ucn_award::is_valid() const
{
    return true;
}

bool ucn_award::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool ucn_award::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool ucn_award::from_data(reader& source)
{
    reset();
    height = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    return result;
}

data_chunk ucn_award::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void ucn_award::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void ucn_award::to_data(writer& sink) const
{
    sink.write_8_bytes_little_endian(height);
}

uint64_t ucn_award::serialized_size() const
{
    //uint64_t size = 8;
    return 8;
}

std::string ucn_award::to_string() const
{
    std::ostringstream ss;
    ss << "\t height = " << height << "\n";

    return ss.str();
}
uint64_t ucn_award::get_height() const
{
    return height;
}

void ucn_award::set_height(uint64_t height)
{
    this->height = height;
}

} // namspace chain
} // namspace libbitcoin
