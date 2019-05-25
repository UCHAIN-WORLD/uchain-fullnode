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
#include <UChain/network/sessions/session_inbound.hpp>

#include <cstddef>
#include <functional>
#include <UChain/coin.hpp>
#include <UChain/network/connector.hpp>
#include <UChain/network/p2p.hpp>
#include <UChain/network/protocols/protocol_address.hpp>
#include <UChain/network/protocols/protocol_ping.hpp>

namespace libbitcoin
{
namespace network
{

#define CLASS session_inbound

using namespace std::placeholders;

session_inbound::session_inbound(p2p &network)
    : session(network, true, true),
      network_{network},
      CONSTRUCT_TRACK(session_inbound)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void session_inbound::start(result_handler handler)
{
    if (settings_.inbound_port == 0 || settings_.inbound_connections == 0)
    {
        log::info(LOG_NETWORK)
            << "Not configured for accepting incoming connections.";
        handler(error::success);
        return;
    }

    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_inbound::handle_started(const code &ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    const auto accept = create_acceptor();
    const auto port = settings_.inbound_port;

    // START LISTENING ON PORT
    accept->listen(port, BIND2(start_accept, _1, accept));

    // This is the end of the start sequence.
    handler(error::success);
}

// Accept sequence.
// ----------------------------------------------------------------------------

void session_inbound::start_accept(const code &ec, acceptor::ptr accept)
{
    if (stopped())
    {
        log::trace(LOG_NETWORK)
            << "Suspended inbound connection.";
        return;
    }

    if (ec)
    {
        log::error(LOG_NETWORK)
            << "Error starting listener: " << ec.message();
        network_.stop();
        return;
    }

    // ACCEPT THE NEXT INCOMING CONNECTION
    accept->accept(BIND3(handle_accept, _1, _2, accept));
}

void session_inbound::handle_accept(const code &ec, channel::ptr channel,
                                    acceptor::ptr accept)
{
    if (stopped())
    {
        log::trace(LOG_NETWORK)
            << "Suspended inbound connection.";
        return;
    }

    start_accept(error::success, accept);

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure accepting connection: " << ec.message();
        return;
    }

    if (blacklisted(channel->authority()))
    {
        log::trace(LOG_NETWORK)
            << "Rejected inbound connection from ["
            << channel->authority() << "] due to blacklisted address.";
        channel->stop(error::accept_failed);
        return;
    }

    connection_count(BIND2(handle_connection_count, _1, channel));
}

void session_inbound::handle_connection_count(size_t connections,
                                              channel::ptr channel)
{
    const auto connection_limit = settings_.inbound_connections +
                                  settings_.outbound_connections;

    if (connections >= connection_limit)
    {
        log::trace(LOG_NETWORK)
            << "Rejected inbound connection from ["
            << channel->authority() << "] due to connection limit.";
        return;
    }

    log::trace(LOG_NETWORK)
        << "Connected inbound channel [" << channel->authority() << "]";

    register_channel(channel,
                     BIND2(handle_channel_start, _1, channel),
                     BIND1(handle_channel_stop, _1));
}

void session_inbound::handle_channel_start(const code &ec,
                                           channel::ptr channel)
{
    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Inbound channel failed to start [" << channel->authority()
            << "] " << ec.message();
        channel->stop(ec);
        return;
    }

    attach_protocols(channel);
};

void session_inbound::attach_protocols(channel::ptr channel)
{
    attach<protocol_ping>(channel)->do_subscribe()->start();
    attach<protocol_address>(channel)->do_subscribe()->start();
}

void session_inbound::handle_channel_stop(const code &ec)
{
    log::trace(LOG_NETWORK)
        << "Inbound channel stopped: " << ec.message();
}

} // namespace network
} // namespace libbitcoin
