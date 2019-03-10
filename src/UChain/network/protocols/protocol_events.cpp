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
#include <UChain/network/protocols/protocol_events.hpp>

#include <functional>
#include <string>
#include <UChain/bitcoin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/p2p.hpp>
#include <UChain/network/protocols/protocol.hpp>

namespace libbitcoin
{
namespace network
{

#define CLASS protocol_events

using namespace std::placeholders;

protocol_events::protocol_events(p2p &network, channel::ptr channel,
                                 const std::string &name)
    : protocol(network, channel, name)
{
}

// Properties.
// ----------------------------------------------------------------------------

bool protocol_events::stopped()
{
    return !handler_.load();
}

// Start.
// ----------------------------------------------------------------------------

void protocol_events::start(event_handler handler)
{
    handler_.store(handler);
    SUBSCRIBE_STOP1(handle_stopped, _1);
    if (channel_stopped())
        set_event(error::channel_stopped);
}

// Stop.
// ----------------------------------------------------------------------------

void protocol_events::handle_stopped(const code &ec)
{
    log::trace(LOG_NETWORK)
        << "Stop protocol_" << name() << " on [" << authority() << "] "
        << ec.message();

    // Event handlers can depend on this code for channel stop.
    set_event(error::channel_stopped);
}

// Set Event.
// ----------------------------------------------------------------------------

void protocol_events::set_event(const code &ec)
{
    auto handler = handler_.load();
    if (!handler)
        return;

    if (ec == (code)error::channel_stopped)
        handler_.store(nullptr);

    handler(ec);
}

// Send Handler.
// ----------------------------------------------------------------------------

void protocol_events::handle_send(const code &ec, const std::string &command)
{
    if (stopped())
        return;

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure sending '" << command << "' to [" << authority()
            << "] " << ec.message();
        stop(ec);
        return;
    }
}

} // namespace network
} // namespace libbitcoin
