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
#include <UChainService/txs/uid/blockchain_uid.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>

namespace libbitcoin
{
namespace chain
{

blockchain_uid::blockchain_uid()
{
    reset();
}
blockchain_uid::blockchain_uid(uint32_t version, const output_point &tx_point,
                               uint64_t height, uint32_t status, const uid_detail &uid) : version_(version), tx_point_(tx_point), height_(height), status_(status), uid_(uid)
{
}

blockchain_uid blockchain_uid::factory_from_data(const data_chunk &data)
{
    blockchain_uid instance;
    instance.from_data(data);
    return instance;
}

blockchain_uid blockchain_uid::factory_from_data(std::istream &stream)
{
    blockchain_uid instance;
    instance.from_data(stream);
    return instance;
}

blockchain_uid blockchain_uid::factory_from_data(reader &source)
{
    blockchain_uid instance;
    instance.from_data(source);
    return instance;
}
bool blockchain_uid::is_valid() const
{
    return true;
}

void blockchain_uid::reset()
{
    version_ = 0;
    tx_point_ = output_point();
    height_ = 0;
    status_ = address_invalid;
    uid_ = uid_detail();
}

bool blockchain_uid::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool blockchain_uid::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool blockchain_uid::from_data(reader &source)
{
    reset();

    version_ = source.read_4_bytes_little_endian();
    tx_point_.from_data(source);
    height_ = source.read_8_bytes_little_endian();
    status_ = source.read_4_bytes_little_endian();
    uid_.from_data(source);

    return true;
}

data_chunk blockchain_uid::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void blockchain_uid::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void blockchain_uid::to_data(writer &sink) const
{
    sink.write_4_bytes_little_endian(version_);
    tx_point_.to_data(sink);
    sink.write_8_bytes_little_endian(height_);
    sink.write_4_bytes_little_endian(status_);
    uid_.to_data(sink);
}

uint64_t blockchain_uid::serialized_size() const
{
    return 4 + tx_point_.serialized_size() + 8 + 4 + uid_.serialized_size();
}

#ifdef UC_DEBUG
std::string blockchain_uid::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
       << "\t tx_point = " << tx_point_.to_string() << "\n"
       << "\t height = " << height_ << "\n"
       << "\t status = " << get_status_string().c_str() << "\n"
       << "\t uid = " << uid_.to_string() << "\n";

    return ss.str();
}

#endif
const uint32_t &blockchain_uid::get_version() const
{
    return version_;
}
void blockchain_uid::set_version(const uint32_t &version_)
{
    this->version_ = version_;
}

const output_point &blockchain_uid::get_tx_point() const
{
    return tx_point_;
}
void blockchain_uid::set_tx_point(const output_point &tx_point_)
{
    this->tx_point_ = tx_point_;
}

const uint64_t &blockchain_uid::get_height() const
{
    return height_;
}
void blockchain_uid::set_height(const uint64_t &height_)
{
    this->height_ = height_;
}

const uid_detail &blockchain_uid::get_uid() const
{
    return uid_;
}
void blockchain_uid::set_uid(const uid_detail &uid_)
{
    this->uid_ = uid_;
}

void blockchain_uid::set_status(const uint32_t &status)
{
    this->status_ = status;
}
const uint32_t &blockchain_uid::get_status() const
{
    return this->status_;
}

std::string blockchain_uid::get_status_string() const
{
    std::string strStatus;
    switch (this->status_)
    {
    case address_invalid:
        strStatus = "invalid";
        break;
    case address_current:
        strStatus = "current";
        break;
    case address_history:
        strStatus = "history";
        break;
    }

    return strStatus;
}
} // namespace chain
} // namespace libbitcoin
