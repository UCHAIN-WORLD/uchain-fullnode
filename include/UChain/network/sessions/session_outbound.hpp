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
#ifndef UC_NETWORK_SESSION_OUTBOUND_HPP
#define UC_NETWORK_SESSION_OUTBOUND_HPP

#include <cstddef>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/sessions/session_batch.hpp>
#include <UChain/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/// Outbound connections session, thread safe.
class BCT_API session_outbound
  : public session_batch, track<session_outbound>
{
public:
    typedef std::shared_ptr<session_outbound> ptr;

    /// Construct an instance.
    session_outbound(p2p& network);

    /// Start the session.
    void start(result_handler handler) override;

protected:
    /// Override to attach specialized protocols upon channel start.
    virtual void attach_protocols(channel::ptr channel);
    void delay_new_connect(connector::ptr connect);

    void delay_reseeding();

private:
    void new_connection(connector::ptr connect);
    void handle_started(const code& ec, result_handler handler);
    void handle_connect(const code& ec, channel::ptr channel,
        connector::ptr connect);

    void handle_channel_stop(const code& ec, connector::ptr connect,
        channel::ptr channel);
    void handle_channel_start(const code& ec, connector::ptr connect,
        channel::ptr channel);

    std::atomic_int outbound_counter;
    std::atomic_bool in_reseeding; //to mark if the re-seeding timer is active
    p2p& network__;
};

} // namespace network
} // namespace libbitcoin

#endif
