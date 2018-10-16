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
#include <UChainService/txs/asset.hpp>
#include <UChainService/txs/variant.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

asset::asset()
{
    reset();
}

asset::asset(const std::string& from_uid, const std::string& to_uid)
    : version(UID_ATTACH_VERIFY_VERSION)
    , type(0) //asset_type::attach_none;
    , touid(to_uid)
    , fromuid(from_uid)
{
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, attach);
}

asset asset::factory_from_data(const data_chunk& data)
{
    asset instance;
    instance.from_data(data);
    return instance;
}

asset asset::factory_from_data(std::istream& stream)
{
    asset instance;
    instance.from_data(stream);
    return instance;
}

asset asset::factory_from_data(reader& source)
{
    asset instance;
    instance.from_data(source);
    return instance;
}

void asset::reset()
{
    version = 0;
    type = 0; //asset_type::attach_none;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, attach);
    touid = "";
    fromuid = "";
}

bool asset::is_valid() const
{
    if (!is_valid_type()) {
        return false;
    }
    auto visitor = is_valid_visitor();
    return boost::apply_visitor(visitor, attach);
}

bool asset::is_valid_type() const
{
    return ((UCN_TYPE == type)
        || (TOKEN_TYPE == type)
        || (TOKEN_CERT_TYPE == type)
        || (TOKEN_CARD_TYPE == type)
        || (MESSAGE_TYPE == type)
        || (UCN_AWARD_TYPE == type)
        || (UID_TYPE == type));
}


bool asset::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset::from_data(reader& source)
{
    reset();

    version = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
        type = source.read_4_bytes_little_endian();

    if (result && version == UID_ATTACH_VERIFY_VERSION) {
            touid = source.read_string();
            fromuid = source.read_string();
    }

    result = static_cast<bool>(source);
    if (result && is_valid_type()) {
        switch(type) {
            case UCN_TYPE:
            {
                attach = ucn();
                break;
            }
            case UCN_AWARD_TYPE:
            {
                attach = ucn_award();
                break;
            }
            case TOKEN_TYPE:
            {
                attach = token();
                break;
            }
            case TOKEN_CERT_TYPE:
            {
                attach = token_cert();
                break;
            }
            case TOKEN_CARD_TYPE:
            {
                attach = token_card();
                break;
            }
            case MESSAGE_TYPE:
            {
                attach = blockchain_message();
                break;
            }
            case UID_TYPE:
            {
                attach = uid();
                break;
            }
        }

        auto visitor = from_data_visitor(source);
        result = boost::apply_visitor(visitor, attach);
    }
    else {
        result = false;
        reset();
    }

    return result;
}

data_chunk asset::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void asset::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(version);
    sink.write_4_bytes_little_endian(type);
    if (version == UID_ATTACH_VERIFY_VERSION) {
        sink.write_string(touid);
        sink.write_string(fromuid);
    }
    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, attach);
}

uint64_t asset::serialized_size() const
{
    uint64_t size = 0;
    if(version == UID_ATTACH_VERIFY_VERSION) {
        size = 4 + 4 + (touid.size() + 1) + (fromuid.size() + 1);
    }
    else {
        size = 4 + 4;
    }

    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, attach);

    return size;
}

std::string asset::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version << "\n"
        << "\t type = " << type << "\n";
    if (version == UID_ATTACH_VERIFY_VERSION) {
        ss << "\t fromuid = " << fromuid << "\n"
            << "\t touid = " << touid << "\n";
    }
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, attach);

    return ss.str();
}

uint32_t asset::get_version() const
{
    return version;
}
void asset::set_version(uint32_t version)
{
     this->version = version;
}

uint32_t asset::get_type() const
{
    return type;
}
void asset::set_type(uint32_t type)
{
     this->type = type;
}

std::string asset::get_to_uid() const
{
    return touid;
}
void asset::set_to_uid(const std::string& uid)
{
    this->touid = uid;
}

std::string asset::get_from_uid() const
{
    return fromuid;
}
void asset::set_from_uid(const std::string& uid)
{
     this->fromuid = uid;
}

asset::asset_data_type& asset::get_attach()
{
    return this->attach;
}
const asset::asset_data_type& asset::get_attach() const
{
    return this->attach;
}

} // namspace chain
} // namspace libbitcoin
