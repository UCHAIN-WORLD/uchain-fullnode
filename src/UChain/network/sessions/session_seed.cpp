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
#include <UChain/network/sessions/session_seed.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <UChain/bitcoin.hpp>
#include <UChain/network/p2p.hpp>
#include <UChain/network/protocols/protocol_ping.hpp>
#include <UChain/network/protocols/protocol_seed.hpp>
#include <UChain/network/proxy.hpp>
#include <UChain/bitcoin/config/authority.hpp>

namespace libbitcoin
{
namespace network
{

#define CLASS session_seed
#define NAME "session_seed"

using namespace std::placeholders;
session_seed::session_seed(p2p &network)
    : session(network, true, false),
      CONSTRUCT_TRACK(session_seed),
      network_{network}
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void session_seed::start(result_handler handler)
{
    if (settings_.host_pool_capacity == 0)
    {
        log::info(LOG_NETWORK)
            << "Not configured to populate an address pool.";
        handler(error::success);
        return;
    }

    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_seed::restart(result_handler handler)
{
    handle_started(error::success, handler);
}

void session_seed::handle_started(const code &ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    address_count(BIND2(handle_count, _1, handler));
}

void session_seed::handle_count(size_t start_size, result_handler handler)
{
    if (start_size != 0)
    {
        log::debug(LOG_NETWORK)
            << "Seeding is not required because there are "
            << start_size << " cached addresses.";
        handler(error::success);
        return;
    }

    if (settings_.seeds.empty())
    {
        log::error(LOG_NETWORK)
            << "Seeding is required but no seeds are configured.";
        handler(error::operation_failed);
        return;
    }

    // This is NOT technically the end of the start sequence, since the handler
    // is not invoked until seeding operations are complete.
    start_seeding(start_size, create_connector(), handler);
}

// Seed sequence.
// ----------------------------------------------------------------------------

void session_seed::start_seeding(size_t start_size, connector::ptr connect,
                                 result_handler handler)
{
    // When all seeds are synchronized call session_seed::handle_complete.
    auto all = BIND2(handle_complete, start_size, handler);

    // Synchronize each individual seed before calling handle_complete.
    auto each = synchronize(all, settings_.seeds.size(), NAME, true);

    // We don't use parallel here because connect is itself asynchronous.
    for (const auto &seed : settings_.seeds)
        start_seed(seed, connect, each);
}

void session_seed::start_seed(const config::endpoint &seed,
                              connector::ptr connect, result_handler handler)
{
    if (stopped())
    {
        log::debug(LOG_NETWORK)
            << "Suspended seed connection";
        handler(error::channel_stopped);
        return;
    }

    log::info(LOG_NETWORK)
        << "Contacting seed [" << seed << "]";

    // OUTBOUND CONNECT
    connect->connect(seed, BIND4(handle_connect, _1, _2, seed, handler), [this](const asio::endpoint &endpoint) {
        network_.store(config::authority{endpoint}.to_network_address(), [](const code &ec) {});
        log::debug(LOG_NETWORK) << "session seed store," << endpoint;
    });
}

void session_seed::handle_connect(const code &ec, channel::ptr channel,
                                  const config::endpoint &seed, result_handler handler)
{
    if (ec)
    {
        log::info(LOG_NETWORK)
            << "Failure contacting seed [" << seed << "] " << ec.message();
        handler(ec);
        return;
    }

    if (blacklisted(channel->authority()))
    {
        log::debug(LOG_NETWORK)
            << "Seed [" << seed << "] on blacklisted address ["
            << channel->authority() << "]";
        handler(error::address_blocked);
        return;
    }

    log::info(LOG_NETWORK)
        << "Connected seed [" << seed << "] as " << channel->authority();

    register_channel(channel,
                     BIND3(handle_channel_start, _1, channel, handler),
                     BIND1(handle_channel_stop, _1));
}

void session_seed::handle_channel_start(const code &ec, channel::ptr channel,
                                        result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    attach_protocols(channel, handler);
};

void session_seed::attach_protocols(channel::ptr channel,
                                    result_handler handler)
{
    attach<protocol_ping>(channel)->start();
    attach<protocol_seed>(channel)->start(handler);
}

void session_seed::handle_channel_stop(const code &ec)
{
    log::info(LOG_NETWORK)
        << "Seed channel stopped: " << ec.message();
}

// This accepts no error code because individual seed errors are suppressed.
void session_seed::handle_complete(size_t start_size, result_handler handler)
{
    address_count(BIND3(handle_final_count, _1, start_size, handler));

    log::info(LOG_NETWORK)
        << "session_seed complete!";
}

// We succeed only if there is a host count increase.
void session_seed::handle_final_count(size_t current_size, size_t start_size,
                                      result_handler handler)
{
    const auto result = current_size > start_size ? error::success : error::operation_failed;

    // This is the end of the seed sequence.
    handler(result);
}

} // namespace network
} // namespace libbitcoin
