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
#include <UChain/network/acceptor.hpp>

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/network/channel.hpp>
#include <UChain/network/proxy.hpp>
#include <UChain/network/settings.hpp>
#include <UChain/network/socket.hpp>

namespace libbitcoin
{
namespace network
{

#define NAME "acceptor"

using namespace std::placeholders;

static const auto reuse_address = asio::acceptor::reuse_address(true);

acceptor::acceptor(threadpool &pool, const settings &settings)
    : pool_(pool),
      settings_(settings),
      dispatch_(pool, NAME),
      acceptor_(std::make_shared<asio::acceptor>(pool_.service())),
      CONSTRUCT_TRACK(acceptor)
{
}

acceptor::~acceptor()
{
    BITCOIN_ASSERT_MSG(!acceptor_->is_open(), "The acceptor was not stopped.");
}

// Stop sequence.
// ----------------------------------------------------------------------------

// public:
void acceptor::stop()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);

    // This will asynchronously invoke the handler of each pending accept.
    acceptor_->close();
    ///////////////////////////////////////////////////////////////////////////
}

// Listen sequence.
// ----------------------------------------------------------------------------

// public:
// This is hardwired to listen on IPv6.
void acceptor::listen(uint16_t port, result_handler handler)
{
    // This is the end of the listen sequence.
    handler(safe_listen(port));
}

code acceptor::safe_listen(uint16_t port)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);

    if (acceptor_->is_open())
        return error::operation_failed;

    boost_code error;
    asio::endpoint endpoint(asio::tcp::v6(), settings_.inbound_port);

    acceptor_->open(endpoint.protocol(), error);

    if (!error)
        acceptor_->set_option(reuse_address, error);

    if (!error)
        acceptor_->bind(endpoint, error);

    if (!error)
        acceptor_->listen(asio::max_connections, error);

    return error::boost_to_error_code(error);
    ///////////////////////////////////////////////////////////////////////////
}

// Accept sequence.
// ----------------------------------------------------------------------------

// public:
void acceptor::accept(accept_handler handler)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock();

    if (!acceptor_->is_open())
    {
        dispatch_.concurrent(handler, error::service_stopped, nullptr);
        mutex_.unlock();
        //---------------------------------------------------------------------
        return;
    }

    const auto socket = std::make_shared<network::socket>(pool_);
    safe_accept(socket, handler);

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

void acceptor::safe_accept(socket::ptr socket, accept_handler handler)
{
    // Critical Section (external)
    ///////////////////////////////////////////////////////////////////////////
    const auto locked = socket->get_socket();

    // async_accept will not invoke the handler within this function.
    acceptor_->async_accept(locked->get(),
                            std::bind(&acceptor::handle_accept,
                                      shared_from_this(), _1, socket, handler));
    ///////////////////////////////////////////////////////////////////////////
}

void acceptor::handle_accept(const boost_code &ec, socket::ptr socket,
                             accept_handler handler)
{
    // This is the end of the accept sequence.
    if (ec)
        handler(error::boost_to_error_code(ec), nullptr);
    else
        handler(error::success, new_channel(socket));
}

std::shared_ptr<channel> acceptor::new_channel(socket::ptr socket)
{
    return std::make_shared<channel>(pool_, socket, settings_);
}

} // namespace network
} // namespace libbitcoin
