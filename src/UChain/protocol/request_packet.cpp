/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-protocol.
 *
 * UChain-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifdef UC_VERSION4

#include <UChain/protocol/packet.hpp>

#include <memory>
#include <UChain/coin.hpp>
#include <UChain/protocol/interface.pb.h>
#include <UChain/protocol/request_packet.hpp>
#include <UChain/protocol/zmq/msg.hpp>
#include <UChain/protocol/zmq/socket.hpp>

namespace libbitcoin
{
namespace protocol
{

request_packet::request_packet()
    : request_(nullptr)
{
}

std::shared_ptr<request> request_packet::get_request() const
{
    return request_;
}

void request_packet::set_request(std::shared_ptr<request> payload)
{
    request_ = payload;
}

bool request_packet::encode_payload(zmq::message &message) const
{
    if (!request_)
        return false;

    const auto data = request_->SerializeAsString();
    message.append({data.begin(), data.end()});
    return true;
}

bool request_packet::decode_payload(const data_chunk &payload)
{
    const auto data = std::make_shared<request>();

    if (!data->ParseFromString({payload.begin(), payload.end()}))
        return false;

    request_ = data;
    return true;
}

} // namespace protocol
} // namespace libbitcoin

#endif
