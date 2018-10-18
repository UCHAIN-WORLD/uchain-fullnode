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
#include <UChainService/txs/token/blockchain_token.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

blockchain_token::blockchain_token()
{
    reset();
}
blockchain_token::blockchain_token(uint32_t version, const output_point& tx_point,
            uint64_t height, const token_detail& token):
    version_(version), tx_point_(tx_point), height_(height), token_(token)
{
}

blockchain_token blockchain_token::factory_from_data(const data_chunk& data)
{
    blockchain_token instance;
    instance.from_data(data);
    return instance;
}

blockchain_token blockchain_token::factory_from_data(std::istream& stream)
{
    blockchain_token instance;
    instance.from_data(stream);
    return instance;
}

blockchain_token blockchain_token::factory_from_data(reader& source)
{
    blockchain_token instance;
    instance.from_data(source);
    return instance;
}
bool blockchain_token::is_valid() const
{
    return true;
}

void blockchain_token::reset()
{
    version_ = 0;
    tx_point_ = output_point();
    height_ = 0;
    token_ = token_detail();
}

bool blockchain_token::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool blockchain_token::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool blockchain_token::from_data(reader& source)
{
    reset();

    version_ = source.read_4_bytes_little_endian();
    tx_point_.from_data(source);
    height_ = source.read_8_bytes_little_endian();
    token_.from_data(source);

    return true;
}

data_chunk blockchain_token::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void blockchain_token::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void blockchain_token::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(version_);
    tx_point_.to_data(sink);
    sink.write_8_bytes_little_endian(height_);
    token_.to_data(sink);
}

uint64_t blockchain_token::serialized_size() const
{
    return 4 + tx_point_.serialized_size() + 8 + token_.serialized_size();
}

#ifdef UC_DEBUG
std::string blockchain_token::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
        << "\t tx_point = " << tx_point_.to_string() << "\n"
        << "\t height = " << height_ << "\n"
        << "\t token = " << token_.to_string() << "\n";

    return ss.str();
}

#endif
const uint32_t& blockchain_token::get_version() const
{
    return version_;
}
void blockchain_token::set_version(const uint32_t& version_)
{
     this->version_ = version_;
}

const output_point& blockchain_token::get_tx_point() const
{
    return tx_point_;
}
void blockchain_token::set_tx_point(const output_point& tx_point_)
{
     this->tx_point_ = tx_point_;
}

const uint64_t& blockchain_token::get_height() const
{
    return height_;
}
void blockchain_token::set_height(const uint64_t& height_)
{
     this->height_ = height_;
}

const token_detail& blockchain_token::get_token() const
{
    return token_;
}
void blockchain_token::set_token(const token_detail& token_)
{
     this->token_ = token_;
}



} // namspace chain
} // namspace libbitcoin
