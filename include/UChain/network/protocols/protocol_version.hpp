/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#ifndef UC_NETWORK_PROTOCOL_VERSION_HPP
#define UC_NETWORK_PROTOCOL_VERSION_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <UChain/bitcoin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/protocols/protocol_timer.hpp>
#include <UChain/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

class BCT_API protocol_version
  : public protocol_timer, track<protocol_version>
{
public:
    typedef std::shared_ptr<protocol_version> ptr;

    /**
     * Construct a version protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     */
    protocol_version(p2p& network, channel::ptr channel);

    /**
     * Start the protocol.
     * @param[in]  handler  Invoked upon stop or receipt of version and verack.
     */
    virtual void start(event_handler handler);

protected:
    void handle_complete(const code& ec);
    /// Override to vary the version message.
    virtual void send_version(const message::version& self);

private:
    static message::version version_factory(
        const config::authority& authority, const settings& settings,
        uint64_t nonce, size_t height);

    void handle_version_sent(const code& ec);
    void handle_verack_sent(const code& ec);

    bool handle_receive_version(const code& ec, message::version::ptr version);
    bool handle_receive_verack(const code& ec, message::verack::ptr);

    p2p& network_;
    std::function<void(const code& ec)> complete_handler_;
};

} // namespace network
} // namespace libbitcoin

#endif

