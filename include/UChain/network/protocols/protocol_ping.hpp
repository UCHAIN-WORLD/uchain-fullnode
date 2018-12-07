/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_NETWORK_PROTOCOL_PING_HPP
#define UC_NETWORK_PROTOCOL_PING_HPP

#include <cstdint>
#include <memory>
#include <UChain/bitcoin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/protocols/protocol_timer.hpp>
#include <UChain/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Ping-pong protocol.
 * Attach this to a channel immediately following handshake completion.
 */
class BCT_API protocol_ping
  : public protocol_timer, track<protocol_ping>
{
public:
    typedef std::shared_ptr<protocol_ping> ptr;

    /**
     * Construct a ping protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     */
    protocol_ping(p2p& network, channel::ptr channel);

    ptr do_subscribe();

    /**
     * Start the protocol.
     */
    virtual void start();

    void handle_or_not(uint64_t nonce);

private:
    void send_ping(const code& ec);
    void test_call_handler(const code& ec);
    bool handle_receive_ping(const code& ec, message::ping::ptr message);
    bool handle_receive_pong(const code& ec, message::pong::ptr message,
        uint64_t nonce);

    const settings& settings_;
};

} // namespace network
} // namespace libbitcoin

#endif
