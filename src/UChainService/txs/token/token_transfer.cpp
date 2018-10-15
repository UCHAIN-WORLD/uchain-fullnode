/**
 * Copyright (c) 2011-2018 UChain developers 
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
#include <UChainService/txs/token/token_transfer.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

bool token_balances::operator< (const token_balances& other) const
{
    typedef std::tuple<std::string, std::string, uint64_t, uint64_t> cmp_tuple;
    return cmp_tuple(symbol, address, unspent_token, locked_token)
        < cmp_tuple(other.symbol, other.address, other.unspent_token, other.locked_token);
}

token_transfer::token_transfer()
{
    reset();
}
token_transfer::token_transfer(const std::string& symbol, uint64_t quantity):
    symbol(symbol),quantity(quantity)
{

}
token_transfer token_transfer::factory_from_data(const data_chunk& data)
{
    token_transfer instance;
    instance.from_data(data);
    return instance;
}

token_transfer token_transfer::factory_from_data(std::istream& stream)
{
    token_transfer instance;
    instance.from_data(stream);
    return instance;
}

token_transfer token_transfer::factory_from_data(reader& source)
{
    token_transfer instance;
    instance.from_data(source);
    return instance;
}

bool token_transfer::is_valid() const
{
    return !(symbol.empty()
            || quantity==0
            || symbol.size()+1 > TOKEN_TRANSFER_SYMBOL_FIX_SIZE);
}

void token_transfer::reset()
{
    symbol = "";
    quantity = 0;
}

bool token_transfer::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool token_transfer::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool token_transfer::from_data(reader& source)
{
    reset();
    symbol = source.read_string();
    quantity = source.read_8_bytes_little_endian();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk token_transfer::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void token_transfer::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void token_transfer::to_data(writer& sink) const
{
    sink.write_string(symbol);
    sink.write_8_bytes_little_endian(quantity);
}

uint64_t token_transfer::serialized_size() const
{
    size_t len = symbol.size() + 8 + 1;
    return std::min(len, TOKEN_TRANSFER_FIX_SIZE);
}

std::string token_transfer::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
        << "\t quantity = " << quantity << "\n";

    return ss.str();
}

const std::string& token_transfer::get_symbol() const
{
    return symbol;
}
void token_transfer::set_symbol(const std::string& symbol)
{
     size_t len = std::min(symbol.size()+1, TOKEN_TRANSFER_SYMBOL_FIX_SIZE);
     this->symbol = symbol.substr(0, len);
}

uint64_t token_transfer::get_quantity() const
{
    return quantity;
}
void token_transfer::set_quantity(uint64_t quantity)
{
     this->quantity = quantity;
}


} // namspace chain
} // namspace libbitcoin
