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

#ifndef UC_PROTOCOL_PACKET
#define UC_PROTOCOL_PACKET

#include <memory>
#include <UChain/coin.hpp>
#include <UChain/protocol/define.hpp>
#include <UChain/protocol/zmq/msg.hpp>

namespace libbitcoin
{
namespace protocol
{

class BCP_API packet
{
  public:
    packet();

    const data_chunk origin() const;
    const data_chunk destination() const;
    void set_destination(const data_chunk &destination);

    bool receive(zmq::socket &socket);
    ////bool receive(const std::shared_ptr<zmq::socket>& socket);
    bool send(zmq::socket &socket);
    ////bool send(const std::shared_ptr<zmq::socket>& socket);

  protected:
    virtual bool encode_payload(zmq::message &message) const = 0;
    virtual bool decode_payload(const data_chunk &payload) = 0;

  private:
    data_chunk origin_;
    data_chunk destination_;
};

} // namespace protocol
} // namespace libbitcoin

#endif
