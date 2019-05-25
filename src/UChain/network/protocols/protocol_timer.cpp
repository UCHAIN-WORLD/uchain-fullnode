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
#include <UChain/network/protocols/protocol_timer.hpp>

#include <functional>
#include <memory>
#include <string>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/p2p.hpp>
#include <UChain/network/protocols/protocol_events.hpp>

namespace libbitcoin
{
namespace network
{

#define CLASS protocol_timer
using namespace std::placeholders;

protocol_timer::protocol_timer(p2p &network, channel::ptr channel,
                               bool perpetual, const std::string &name)
    : protocol_events(network, channel, name),
      perpetual_(perpetual)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

// protected:
void protocol_timer::start(const asio::duration &timeout,
                           event_handler handle_event)
{
    // The deadline timer is thread safe.
    timer_ = std::make_shared<deadline>(pool(), timeout);
    protocol_events::start(BIND2(handle_notify, _1, handle_event));
    reset_timer();
}

void protocol_timer::handle_notify(const code &ec, event_handler handler)
{
    if (ec == (code)error::channel_stopped)
        timer_->stop();

    handler(ec);
}

// Timer.
// ----------------------------------------------------------------------------

// protected:
void protocol_timer::reset_timer()
{
    if (stopped())
        return;

    timer_->start(BIND1(handle_timer, _1));
    if (stopped())
    {
        timer_->stop();
        return;
    }
}

void protocol_timer::handle_timer(const code &ec)
{
    if (stopped())
    {
        return;
    }

    // The handler completes before the timer is reset.
    set_event(error::channel_timeout);

    // A perpetual timer resets itself until the channel is stopped.
    if (perpetual_)
        reset_timer();
}

} // namespace network
} // namespace libbitcoin
