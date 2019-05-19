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
#ifndef UC_NETWORK_PROTOCOL_EVENTS_HPP
#define UC_NETWORK_PROTOCOL_EVENTS_HPP

#include <string>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/protocols/protocol.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Base class for stateful protocol implementation, thread and lock safe.
 */
class BCT_API protocol_events
  : public protocol
{
protected:

    /**
     * Construct a protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     * @param[in]  name      The instance name for logging purposes.
     */
    protocol_events(p2p& network, channel::ptr channel,
        const std::string& name);

    /**
     * Start the protocol.
     * The event handler may be invoked one or more times.
     * @param[in]  handler  The handler to call at each completion event.
     */
    virtual void start(event_handler handler);

    /**
     * Invoke the event handler.
     * @param[in]  ec  The error code of the preceding operation.
     */
    virtual void set_event(const code& ec);

    /**
     * Determine if the event handler has been cleared.
     */
    virtual bool stopped();

protected:
    void handle_send(const code& ec, const std::string& command);

private:
    void handle_started(completion_handler handler);
    void handle_stopped(const code& ec);
    void do_set_event(const code& ec);

    bc::atomic<event_handler> handler_;
};

} // namespace network
} // namespace libbitcoin

#endif
