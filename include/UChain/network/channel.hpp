/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef UC_NETWORK_CHANNEL_HPP
#define UC_NETWORK_CHANNEL_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <string>
#include <UChain/bitcoin.hpp>
#include <UChain/network/const_buffer.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/proxy.hpp>
#include <UChain/network/message_subscriber.hpp>
#include <UChain/network/settings.hpp>
#include <UChain/network/socket.hpp>

namespace libbitcoin {
namespace network {

/// A concrete proxy with timers and state, mostly thread safe.
class BCT_API channel
  : public proxy, track<channel>
{
public:
    typedef std::shared_ptr<channel> ptr;

    /// Construct an instance.
    channel(threadpool& pool, socket::ptr socket, const settings& settings);

    void start(result_handler handler) override;

    virtual bool notify() const;
    virtual void set_notify(bool value);

    virtual uint64_t nonce() const;
    virtual void set_nonce(uint64_t value);

    void set_protocol_start_handler(std::function<void()> handler);

    void invoke_protocol_start_handler(const code& ec);

protected:
    virtual void handle_activity();
    virtual void handle_stopping();

private:
    void do_start(const code& ec, result_handler handler);

    void start_expiration();
    void handle_expiration(const code& ec);

    void start_inactivity();
    void handle_inactivity(const code& ec);

    bool notify_;
    uint64_t nonce_;
    deadline::ptr expiration_;
    deadline::ptr inactivity_;
    std::function<void()> protocol_start_handler_;
    upgrade_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
