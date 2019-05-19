/**
 * Copyright (c) 2011-2018 libbitcoin developers 
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
#ifndef UC_NETWORK_SESSION_BATCH_HPP
#define UC_NETWORK_SESSION_BATCH_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/connector.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/sessions/session.hpp>
#include <UChain/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/// Intermediate base class for adding batch connect sequence.
class BCT_API session_batch
  : public session
{
protected:

    /// Construct an instance.
    session_batch(p2p& network, bool persistent);

    /// Create a channel from the configured number of concurrent attempts.
    virtual void connect(connector::ptr connect, channel_handler handler);

private:
    typedef std::atomic<size_t> atomic_counter;
    typedef std::shared_ptr<atomic_counter> atomic_counter_ptr;
    typedef std::shared_ptr<upgrade_mutex> upgrade_mutex_ptr;

    void converge(const code& ec, channel::ptr channel,
        atomic_counter_ptr counter, upgrade_mutex_ptr mutex,
        channel_handler handler);

    // Connect sequence
    void new_connect(connector::ptr connect, atomic_counter_ptr counter,
        channel_handler handler);
    void start_connect(const code& ec, const authority& host,
        connector::ptr connect, atomic_counter_ptr counter,
        channel_handler handler);
    void handle_connect(const code& ec, channel::ptr channel,
        const authority& host, connector::ptr connect,
        atomic_counter_ptr counter, std::size_t count, channel_handler handler);

    const size_t batch_size_;
};

} // namespace network
} // namespace libbitcoin

#endif
