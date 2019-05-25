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
#include <UChainService/txs/uid/uid_detail.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin
{
namespace chain
{

uid_detail::uid_detail()
{
    reset();
}

uid_detail::uid_detail(
    const std::string &symbol, const std::string &address)
    : symbol(symbol), address(address)
{
}

uid_detail uid_detail::factory_from_data(const data_chunk &data)
{
    uid_detail instance;
    instance.from_data(data);
    return instance;
}

uid_detail uid_detail::factory_from_data(std::istream &stream)
{
    uid_detail instance;
    instance.from_data(stream);
    return instance;
}

uid_detail uid_detail::factory_from_data(reader &source)
{
    uid_detail instance;
    instance.from_data(source);
    return instance;
}
bool uid_detail::is_valid() const
{
    return !(symbol.empty() || count_size() > UID_DETAIL_FIX_SIZE);
}

void uid_detail::reset()
{
    symbol = "";
    address = "";
}

bool uid_detail::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool uid_detail::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool uid_detail::from_data(reader &source)
{
    reset();

    symbol = source.read_string();
    address = source.read_string();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk uid_detail::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void uid_detail::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void uid_detail::to_data(writer &sink) const
{
    sink.write_string(symbol);
    sink.write_string(address);
}

uint64_t uid_detail::serialized_size() const
{
    size_t len = count_size();
    return std::min(UID_DETAIL_FIX_SIZE, len);
}

uint32_t uid_detail::count_size() const
{
    return symbol.size() + address.size() + 2;
}

bool uid_detail::operator<(const uid_detail &other) const
{
    auto ret = 0;
    if ((ret = symbol.compare(other.symbol)) < 0 || (ret == 0 && address.compare(other.address) < 0))
    {
        return true;
    }

    return false;
}

std::string uid_detail::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
       << "\t address = " << address << "\n";

    return ss.str();
}

void uid_detail::to_json(std::ostream &output)
{
    minijson::object_writer json_writer(output);
    json_writer.write("symbol", symbol);
    json_writer.write("address", address);
    json_writer.close();
}

const std::string &uid_detail::get_symbol() const
{
    return symbol;
}

void uid_detail::set_symbol(const std::string &symbol)
{
    size_t len = std::min(symbol.size() + 1, UID_DETAIL_SYMBOL_FIX_SIZE);
    this->symbol = symbol.substr(0, len);
}

const std::string &uid_detail::get_address() const
{
    return address;
}

void uid_detail::set_address(const std::string &address)
{
    size_t len = std::min(address.size() + 1, UID_DETAIL_ADDRESS_FIX_SIZE);
    this->address = address.substr(0, len);
}

} // namespace chain
} // namespace libbitcoin
