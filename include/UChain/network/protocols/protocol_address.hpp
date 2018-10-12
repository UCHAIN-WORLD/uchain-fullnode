/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#ifndef UC_NETWORK_PROTOCOL_ADDRESS_HPP
#define UC_NETWORK_PROTOCOL_ADDRESS_HPP

#include <memory>
#include <UChain/bitcoin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Address protocol.
 * Attach this to a channel immediately following handshake completion.
 */
class BCT_API protocol_address
  : public protocol_events, track<protocol_address>
{
public:
    typedef std::shared_ptr<protocol_address> ptr;

    /**
     * Construct an address protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     */
    protocol_address(p2p& network, channel::ptr channel);

    ptr do_subscribe();

    /**
     * Start the protocol.
     */
    virtual void start();

private:
    void handle_stop(const code& ec);
    void handle_store_addresses(const code& ec, message::address::ptr message);
    void remove_useless_address(message::address::ptr& message);
    bool handle_receive_address(const code& ec, message::address::ptr address);
    bool handle_receive_get_address(const code& ec,
        message::get_address::ptr message);

    p2p& network_;
    message::address self_;
};

} // namespace network
} // namespace libbitcoin

#endif
