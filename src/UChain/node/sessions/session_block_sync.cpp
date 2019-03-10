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
#include <UChain/node/sessions/session_block_sync.hpp>

#include <cstddef>
#include <memory>
#include <UChain/blockchain.hpp>
#include <UChain/network.hpp>
#include <UChain/node/define.hpp>
#include <UChain/node/protocols/protocol_block_sync.hpp>
#include <UChain/node/protocols/protocol_version_quiet.hpp>
#include <UChain/node/settings.hpp>
#include <UChain/node/utility/header_queue.hpp>
#include <UChain/node/utility/reservation.hpp>

namespace libbitcoin
{
namespace node
{

#define CLASS session_block_sync
#define NAME "session_block_sync"

using namespace blockchain;
using namespace config;
using namespace network;
using namespace std::placeholders;

// The interval in which all-channel block download performance is tested.
static const asio::seconds regulator_interval(5);

session_block_sync::session_block_sync(p2p &network, header_queue &hashes,
                                       simple_chain &chain, const settings &settings)
    : session_batch(network, false),
      blockchain_(chain),
      reservations_count_{0},
      settings_(settings),
      reservations_(hashes, chain, settings),
      CONSTRUCT_TRACK(session_block_sync)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void session_block_sync::start(result_handler handler)
{
    // TODO: create session_timer base class and pass interval via start.
    timer_ = std::make_shared<deadline>(pool_, regulator_interval);
    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_block_sync::handle_started(const code &ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    // Copy the reservations table.
    const auto table = reservations_.table();

    if (table.empty())
    {
        handler(error::success);
        return;
    }

    // TODO: expose valid block count from reservations and emit here.
    log::info(LOG_NODE)
        << "Getting blocks.";
    const auto connector = create_connector();
    reservations_count_ = table.size();
    const auto complete = synchronize(handler, table.size(), NAME);
    std::function<void(const code &)> func = complete;
    // This is the end of the start sequence.
    for (const auto row : table)
        new_connection(connector, row, [func](const code &ec) {
            func(ec);
        });
    log::info(LOG_NODE)
        << "table size," << table.size();

    ////reset_timer(connector);
}

// Block sync sequence.
// ----------------------------------------------------------------------------

void session_block_sync::new_connection(connector::ptr connect,
                                        reservation::ptr row, result_handler handler)
{
    if (stopped())
    {
        log::debug(LOG_NODE)
            << "Suspending slot (" << row->slot() << ").";
        return;
    }

    log::debug(LOG_NODE)
        << "Starting slot (" << row->slot() << ").";

    // BLOCK SYNC CONNECT
    this->connect(connect,
                  BIND5(handle_connect, _1, _2, connect, row, handler));
}

void session_block_sync::handle_connect(const code &ec, channel::ptr channel,
                                        connector::ptr connect, reservation::ptr row, result_handler handler)
{
    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure connecting slot (" << row->slot() << ") "
            << ec.message();
        if (ec.value() == error::not_satisfied)
        {
            log::debug(LOG_NETWORK) << "session block sync handle connect, not satified";
            handle_complete(ec, channel, connect, row, handler);
            return;
        }
        new_connection(connect, row, handler);
        return;
    }

    log::debug(LOG_NODE)
        << "Connected slot (" << row->slot() << ") ["
        << channel->authority() << "]";

    register_channel(channel,
                     BIND5(handle_channel_start, _1, channel, connect, row, handler),
                     BIND4(handle_channel_stop, _1, connect, row, handler));
}

void session_block_sync::attach_handshake_protocols(channel::ptr channel,
                                                    result_handler handle_started)
{
    attach<protocol_version_quiet>(channel)->start(handle_started);
}

void session_block_sync::handle_channel_start(const code &ec,
                                              channel::ptr channel, connector::ptr connect, reservation::ptr row,
                                              result_handler handler)
{
    // Treat a start failure just like a completion failure.
    if (ec)
    {
        handle_complete(ec, channel, connect, row, handler);
        return;
    }

    attach_protocols(channel, connect, row, handler);
}

void session_block_sync::attach_protocols(channel::ptr channel,
                                          connector::ptr connect, reservation::ptr row, result_handler handler)
{
    attach<protocol_ping>(channel)->start();
    attach<protocol_address>(channel)->start();
    attach<protocol_block_sync>(channel, row)->start(BIND5(handle_complete, _1, channel, connect, row, handler));
}

void session_block_sync::handle_complete(const code &ec, channel::ptr channel,
                                         network::connector::ptr connect, reservation::ptr row,
                                         result_handler handler)
{
    if (!ec)
    {
        timer_->stop();
        scoped_lock lock{mutex_};
        --reservations_count_;
        reservations_.remove(row);
        log::debug(LOG_NODE)
            << "Completed slot (" << row->slot() << ")"
            << ",resd size," << reservations_.size();
        // This is the end of the block sync sequence.
        handler(ec);

        return;
    }

    if (ec.value() == error::not_satisfied)
    {
        scoped_lock lock{mutex_};
        if (reservations_count_ != 1)
        {
            --reservations_count_;
            handler(error::success);
            return;
        }
    }

    // There is no failure scenario, we ignore the result code here.
    new_connection(connect, row, handler);
}

void session_block_sync::handle_channel_stop(const code &ec,
                                             network::connector::ptr connect, reservation::ptr row, result_handler handler)
{
    log::info(LOG_NODE)
        << "Channel stopped on slot (" << row->slot() << ") " << ec.message();
}

// Timer.
// ----------------------------------------------------------------------------

// private:
void session_block_sync::reset_timer(connector::ptr connect)
{
    if (stopped())
        return;

    timer_->start(BIND2(handle_timer, _1, connect));
}

void session_block_sync::handle_timer(const code &ec, connector::ptr connect)
{
    if (stopped())
        return;

    log::debug(LOG_NODE)
        << "Fired session_block_sync timer: " << ec.message();

    ////// TODO: If (total database time as a fn of total time) add a channel.
    ////// TODO: push into reservations_ implementation.
    ////// TODO: add a boolean increment method to the synchronizer and pass here.
    ////const size_t id = reservations_.table().size();
    ////const auto row = std::make_shared<reservation>(reservations_, id);
    ////const synchronizer<result_handler> handler({}, 0, "name", true);

    ////if (add) new_connection(connect, row, handler);

    ////// TODO: drop the slowest channel
    //////If (drop) reservations_.prune();

    reset_timer(connect);
}

} // namespace node
} // namespace libbitcoin
