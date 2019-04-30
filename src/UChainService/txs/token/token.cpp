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
#include <UChainService/txs/token/token.hpp>
#include <UChainService/txs/variant.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <UChainService/txs/token/token_transfer.hpp>
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

token::token()
{
    reset();
}
token::token(uint32_t status, const token_detail &detail) : status(status), data(detail)
{
}
token::token(uint32_t status, const token_transfer &detail) : status(status), data(detail)
{
}
token token::factory_from_data(const data_chunk &data)
{
    token instance;
    instance.from_data(data);
    return instance;
}

token token::factory_from_data(std::istream &stream)
{
    token instance;
    instance.from_data(stream);
    return instance;
}

token token::factory_from_data(reader &source)
{
    token instance;
    instance.from_data(source);
    return instance;
}

void token::reset()
{
    status = 0; //token_status::token_none;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, data);
}

bool token::is_valid() const
{
    auto visitor = is_valid_visitor();
    return boost::apply_visitor(visitor, data);
}

bool token::is_valid_type() const
{
    return ((TOKEN_DETAIL_TYPE == status) || (TOKEN_TRANSFERABLE_TYPE == status));
}

bool token::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool token::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool token::from_data(reader &source)
{
    reset();

    status = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type())
    {
        switch (status)
        {
        case TOKEN_DETAIL_TYPE:
        {
            data = token_detail();
            break;
        }
        case TOKEN_TRANSFERABLE_TYPE:
        {
            data = token_transfer();
            break;
        }
        }
        auto visitor = from_data_visitor(source);
        result = boost::apply_visitor(visitor, data);
    }
    else
    {
        result = false;
        reset();
    }

    return result;
}

data_chunk token::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void token::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void token::to_data(writer &sink) const
{
    sink.write_4_bytes_little_endian(status);

    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, data);
}

uint64_t token::serialized_size() const
{
    uint64_t size = 0;

    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, data);
    return 4 + size;
}

std::string token::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << status << "\n";
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, data);
    return ss.str();
}

uint32_t token::get_status() const
{
    return status;
}
void token::set_status(uint32_t status)
{
    this->status = status;
}
void token::set_data(const token_detail &detail)
{
    this->data = detail;
}
void token::set_data(const token_transfer &detail)
{
    this->data = detail;
}
token::token_data_type &token::get_data()
{
    return this->data;
}
const token::token_data_type &token::get_data() const
{
    return this->data;
}

} // namespace chain
} // namespace libbitcoin
