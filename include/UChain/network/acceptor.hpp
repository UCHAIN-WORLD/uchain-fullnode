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
#ifndef UC_NETWORK_ACCEPTOR_HPP
#define UC_NETWORK_ACCEPTOR_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/settings.hpp>
#include <UChain/network/socket.hpp>

namespace libbitcoin {
namespace network {

/// Create inbound socket connections, thread and lock safe.
class BCT_API acceptor
  : public enable_shared_from_base<acceptor>, track<acceptor>
{
public:
    typedef std::shared_ptr<acceptor> ptr;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, channel::ptr)> accept_handler;

    /// Construct an instance.
    acceptor(threadpool& pool, const settings& settings);

    /// Validate acceptor stopped.
    ~acceptor();

    /// This class is not copyable.
    acceptor(const acceptor&) = delete;
    void operator=(const acceptor&) = delete;

    /// Start the listener on the specified port.
    virtual void listen(uint16_t port, result_handler handler);

    /// Accept the next connection available, until canceled.
    virtual void accept(accept_handler handler);

    /// Cancel the listener and all outstanding accept attempts.
    virtual void stop();

private:
    code safe_listen(uint16_t port);
    void safe_accept(socket::ptr socket, accept_handler handler);
    std::shared_ptr<channel> new_channel(socket::ptr socket);
    void handle_accept(const boost_code& ec, socket::ptr socket,
        accept_handler handler);

    threadpool& pool_;
    const settings& settings_;
    dispatcher dispatch_;
    asio::acceptor_ptr acceptor_;
    mutable shared_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
