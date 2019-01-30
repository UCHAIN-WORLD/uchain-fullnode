/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-server.
 *
 * UChain-server is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChainApp/ucd/server_node.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <UChain/node.hpp>
#include <UChainApp/ucd/config.hpp>
#include <UChainApp/ucd/messages/route.hpp>
#include <UChainApp/ucd/workers/query_worker.hpp>
#include <UChainService/api/rest.hpp>

#include <thread>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#include <shellapi.h>
#endif

namespace libbitcoin {
namespace server {

using namespace std::placeholders;
using namespace bc::chain;
using namespace bc::node;
using namespace bc::protocol;

server_node::server_node(const configuration& configuration)
  : p2p_node(configuration),
    under_blockchain_sync_(true),
    configuration_(configuration),
    authenticator_(*this),
    secure_query_service_(authenticator_, *this, true),
    public_query_service_(authenticator_, *this, false),
    secure_heartbeat_service_(authenticator_, *this, true),
    public_heartbeat_service_(authenticator_, *this, false),
    secure_block_service_(authenticator_, *this, true),
    public_block_service_(authenticator_, *this, false),
    secure_transaction_service_(authenticator_, *this, true),
    public_transaction_service_(authenticator_, *this, false),
    secure_notification_worker_(authenticator_, *this, true),
    public_notification_worker_(authenticator_, *this, false),
    miner_(*this),
    rest_server_(new mgbubble::RestServ(webpage_path_.string().data(), *this, configuration.server.mongoose_listen)),
    push_server_(new mgbubble::WsPushServ(*this, configuration.server.websocket_listen))
{
}

// This allows for shutdown based on destruct without need to call stop.
server_node::~server_node()
{
    server_node::close();
}

// Properties.
// ----------------------------------------------------------------------------

const settings& server_node::server_settings() const
{
    return configuration_.server;
}

// Run sequence.
// ----------------------------------------------------------------------------

void server_node::run(result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    if (!rest_server_->start() || !push_server_->start())
    {
        log::error(LOG_SERVER) << "Http/Websocket server can not start.";
        handler(error::operation_failed);
        return;
    }

    // The handler is invoked on a new thread.
    p2p_node::run(
        std::bind(&server_node::handle_running,
            this, _1, handler));
}

void server_node::handle_running(const code& ec, result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    if (ec)
    {
        handler(ec);
        return;
    }

    if (!start_services())
    {
        handler(error::operation_failed);
        return;
    }

    under_blockchain_sync_.store(false, std::memory_order_relaxed);

    // This is the end of the derived run sequence.
    handler(error::success);
}

// Shutdown.
// ----------------------------------------------------------------------------

bool server_node::stop()
{
    // Suspend new work last so we can use work to clear subscribers.
    return authenticator_.stop() && p2p_node::stop();
}

// This must be called from the thread that constructed this class (see join).
bool server_node::close()
{
    // Invoke own stop to signal work suspension, then close node and join.
    return server_node::stop() && p2p_node::close();
}

/// Get miner.
consensus::miner& server_node::miner()
{
    return miner_;
}

// Notification.
// ----------------------------------------------------------------------------

// Subscribe to address (including stealth) prefix notifications.
void server_node::subscribe_address(const route& reply_to, uint32_t id,
    const binary& prefix_filter, subscribe_type type)
{
    if (reply_to.secure)
        secure_notification_worker_
            .subscribe_address(reply_to, id, prefix_filter, type);
    else
        public_notification_worker_
            .subscribe_address(reply_to, id, prefix_filter, type);
}

// Subscribe to transaction penetration notifications.
void server_node::subscribe_penetration(const route& reply_to, uint32_t id,
    const hash_digest& tx_hash)
{
    if (reply_to.secure)
        secure_notification_worker_
            .subscribe_penetration(reply_to, id, tx_hash);
    else
        public_notification_worker_
            .subscribe_penetration(reply_to, id, tx_hash);
}

// Services.
// ----------------------------------------------------------------------------

bool server_node::start_services()
{
    return
        start_authenticator() && start_query_services() &&
        start_heartbeat_services() && start_block_services() &&
        start_transaction_services();
}

bool server_node::start_authenticator()
{
    const auto& settings = configuration_.server;
    const auto heartbeat_interval = settings.heartbeat_interval_seconds;

    if ((!settings.server_private_key && settings.secure_only) ||
        ((!settings.query_service_enabled || settings.query_workers == 0) &&
        (!settings.heartbeat_service_enabled || heartbeat_interval == 0) &&
        (!settings.block_service_enabled) &&
        (!settings.transaction_service_enabled)))
        return true;

    return authenticator_.start();
}

bool server_node::start_query_services()
{
    const auto& settings = configuration_.server;

    if (!settings.query_service_enabled || settings.query_workers == 0)
        return true;

    // Start secure service, query workers and notification workers if enabled.
    if (settings.server_private_key && (!secure_query_service_.start() ||
        (settings.subscription_limit > 0 && !secure_notification_worker_.start()) ||
        !start_query_workers(true)))
            return false;

    // Start public service, query workers and notification workers if enabled.
    if (!settings.secure_only && (!public_query_service_.start() ||
        (settings.subscription_limit > 0 && !public_notification_worker_.start()) ||
        !start_query_workers(false)))
            return false;

    return true;
}

bool server_node::start_heartbeat_services()
{
    const auto& settings = configuration_.server;

    if (!settings.heartbeat_service_enabled ||
        settings.heartbeat_interval_seconds == 0)
        return true;

    // Start secure service if enabled.
    if (settings.server_private_key && !secure_heartbeat_service_.start())
        return false;

    // Start public service if enabled.
    if (!settings.secure_only && !public_heartbeat_service_.start())
        return false;

    return true;
}

bool server_node::start_block_services()
{
    const auto& settings = configuration_.server;

    if (!settings.block_service_enabled)
        return true;

    // Start secure service if enabled.
    if (settings.server_private_key && !secure_block_service_.start())
        return false;

    // Start public service if enabled.
    if (!settings.secure_only && !public_block_service_.start())
        return false;

    return true;
}

bool server_node::start_transaction_services()
{
    const auto& settings = configuration_.server;

    if (!settings.transaction_service_enabled)
        return true;

    // Start secure service if enabled.
    if (settings.server_private_key && !secure_transaction_service_.start())
        return false;

    // Start public service if enabled.
    if (!settings.secure_only && !public_transaction_service_.start())
        return false;

    return true;
}

// Called from start_query_services.
bool server_node::start_query_workers(bool secure)
{
    auto& server = *this;
    const auto& settings = configuration_.server;

    for (auto count = 0; count < settings.query_workers; ++count)
    {
        auto worker = std::make_shared<query_worker>(authenticator_,
            server, secure);

        if (!worker->start())
            return false;

        subscribe_stop([=](const code&) { worker->stop(); });
    }

    return true;
}

bool server_node::open_ui()
{
#ifdef _WIN32
    const TCHAR szOperation[] = _T("open");
    const TCHAR szAddress[] = _T("http://127.0.0.1:8707");
    HINSTANCE hRslt = ShellExecute(NULL, szOperation,
        szAddress, NULL, NULL, SW_SHOWNORMAL);

    if (hRslt <= (HINSTANCE)HINSTANCE_ERROR)
        log::error("shell execute failed when open UI");
        return false;
    log::info("UI on display");
#endif
    return true;
}

// static
uint32_t server_node::threads_required(const configuration& configuration)
{
    const auto& settings = configuration.server;
    const auto threads = configuration.network.threads;
    const auto heartbeat_interval = settings.heartbeat_interval_seconds;

    // The network/node requires a minimum of one thread.
    uint32_t required = 1;

    if (settings.query_service_enabled && settings.query_workers > 0)
    {
        if (settings.server_private_key)
        {
            ++required;
            required += settings.query_workers;
            required += (settings.subscription_limit > 0 ? 4 : 0);
        }

        if (!settings.secure_only)
        {
            ++required;
            required += settings.query_workers;
            required += (settings.subscription_limit > 0 ? 4 : 0);
        }
    }

    if (settings.heartbeat_service_enabled && heartbeat_interval > 0)
    {
        required += (settings.server_private_key ? 1 : 0);
        required += (settings.secure_only ? 0 : 1);
    }

    if (settings.block_service_enabled)
    {
        required += (settings.server_private_key ? 1 : 0);
        required += (settings.secure_only ? 0 : 1);
    }

    if (settings.transaction_service_enabled)
    {
        required += (settings.server_private_key ? 1 : 0);
        required += (settings.secure_only ? 0 : 1);
    }

    // If any services are enabled increment for the authenticator.
    return required == 1 ? required : required + 1;
}

boost::filesystem::path server_node::webpage_path_ = webpage_path();

} // namespace server
} // namespace libbitcoin
