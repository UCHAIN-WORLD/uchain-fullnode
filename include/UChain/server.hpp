///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-server developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UC_SERVER_HPP
#define UC_SERVER_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <UChain/node.hpp>
#include <UChain/protocol.hpp>
#include <UChain/server/configuration.hpp>
#include <UChain/server/define.hpp>
#include <UChain/server/parser.hpp>
#include <UChain/server/server_node.hpp>
#include <UChain/server/settings.hpp>
#include <UChain/server/version.hpp>
#include <UChain/server/interface/address.hpp>
#include <UChain/server/interface/blockchain.hpp>
#include <UChain/server/interface/protocol.hpp>
#include <UChain/server/interface/transaction_pool.hpp>
#include <UChain/server/messages/message.hpp>
#include <UChain/server/messages/route.hpp>
#include <UChain/server/services/block_service.hpp>
#include <UChain/server/services/heartbeat_service.hpp>
#include <UChain/server/services/query_service.hpp>
#include <UChain/server/services/transaction_service.hpp>
#include <UChain/server/utility/address_key.hpp>
#include <UChain/server/utility/authenticator.hpp>
#include <UChain/server/utility/fetch_helpers.hpp>
#include <UChain/server/workers/notification_worker.hpp>
#include <UChain/server/workers/query_worker.hpp>

#endif
