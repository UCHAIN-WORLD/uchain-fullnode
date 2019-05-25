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
#include <UChainService/txs/asset_data.hpp>
#include <UChainService/txs/variant.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>

#define UCN_TYPE KIND2UINT16(business_kind::ucn)
#define UCN_AWARD_TYPE KIND2UINT16(business_kind::ucn_award)
#define TOKEN_ISSUE_TYPE KIND2UINT16(business_kind::token_issue)
#define TOKEN_TRANSFER_TYPE KIND2UINT16(business_kind::token_transfer)
#define TOKEN_CERT_TYPE KIND2UINT16(business_kind::token_cert)
#define TOKEN_CANDIDATE_TYPE KIND2UINT16(business_kind::candidate)
#define MESSAGE_TYPE KIND2UINT16(business_kind::message)
#define UID_REGISTER_TYPE KIND2UINT16(business_kind::uid_register)
#define UID_TRANSFER_TYPE KIND2UINT16(business_kind::uid_transfer)

namespace libbitcoin
{
namespace chain
{

asset_data asset_data::factory_from_data(const data_chunk &data)
{
    asset_data instance;
    instance.from_data(data);
    return instance;
}

asset_data asset_data::factory_from_data(std::istream &stream)
{
    asset_data instance;
    instance.from_data(stream);
    return instance;
}

asset_data asset_data::factory_from_data(reader &source)
{
    asset_data instance;
    instance.from_data(source);
    return instance;
}

void asset_data::reset()
{
    kind = business_kind::ucn;
    timestamp = 0;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, data);
}
bool asset_data::is_valid() const
{
    return true;
}

bool asset_data::is_valid_type() const
{
    return ((UCN_TYPE == KIND2UINT16(kind)) || (TOKEN_ISSUE_TYPE == KIND2UINT16(kind)) || (TOKEN_TRANSFER_TYPE == KIND2UINT16(kind)) || (TOKEN_CERT_TYPE == KIND2UINT16(kind)) || (TOKEN_CANDIDATE_TYPE == KIND2UINT16(kind))) || (UCN_AWARD_TYPE == KIND2UINT16(kind)) || (MESSAGE_TYPE == KIND2UINT16(kind)) || (UID_REGISTER_TYPE == KIND2UINT16(kind)) || (UID_TRANSFER_TYPE == KIND2UINT16(kind));
}

bool asset_data::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset_data::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset_data::from_data(reader &source)
{
    reset();
    kind = static_cast<business_kind>(source.read_2_bytes_little_endian());
    timestamp = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type())
    {
        switch (KIND2UINT16(kind))
        {
        case UCN_TYPE:
        {
            data = ucn();
            break;
        }
        case UCN_AWARD_TYPE:
        {
            data = ucn_award();
            break;
        }
        case TOKEN_ISSUE_TYPE:
        {
            data = token_detail();
            break;
        }
        case TOKEN_TRANSFER_TYPE:
        {
            data = token_transfer();
            break;
        }
        case TOKEN_CERT_TYPE:
        {
            data = token_cert();
            break;
        }
        case TOKEN_CANDIDATE_TYPE:
        {
            data = candidate();
            break;
        }
        case MESSAGE_TYPE:
        {
            data = blockchain_message();
            break;
        }
        case UID_REGISTER_TYPE:
        {
            data = uid_detail();
            break;
        }
        case UID_TRANSFER_TYPE:
        {
            data = uid_detail();
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

data_chunk asset_data::to_data()
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void asset_data::to_data(std::ostream &stream)
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset_data::to_data(writer &sink)
{
    sink.write_2_bytes_little_endian(KIND2UINT16(kind));
    sink.write_4_bytes_little_endian(timestamp);
    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, data);
}

uint64_t asset_data::serialized_size()
{
    uint64_t size = 2 + 4; // kind and timestamp
    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, data);

    return size;
}

#ifdef UC_DEBUG
std::string asset_data::to_string()
{
    std::ostringstream ss;

    ss << "\t kind = " << KIND2UINT16(kind) << "\n";
    ss << "\t timestamp = " << timestamp << "\n";
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, data);

    return ss.str();
}
#endif

business_kind asset_data::get_kind_value() const
{
    //return KIND2UINT16(kind);
    return kind;
}

const asset_data::asset_data_type &asset_data::get_data() const
{
    return data;
}

uint32_t asset_data::get_timestamp() const
{
    return timestamp;
}

} // namespace chain
} // namespace libbitcoin
