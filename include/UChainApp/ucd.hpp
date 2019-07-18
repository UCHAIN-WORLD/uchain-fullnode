///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2018 libbitcoin-server developers (see COPYING).
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
#include <UChainApp/ucd/config.hpp>
#include <UChainApp/ucd/define.hpp>
#include <UChainApp/ucd/parser.hpp>
#include <UChainApp/ucd/server_node.hpp>
#include <UChainApp/ucd/settings.hpp>
#include <UChainApp/ucd/version.hpp>
#include <UChainApp/ucd/interface/address.hpp>
#include <UChainApp/ucd/interface/blockchain.hpp>
#include <UChainApp/ucd/interface/protocol.hpp>
#include <UChainApp/ucd/interface/tx_pool.hpp>
#include <UChainApp/ucd/messages/message.hpp>
#include <UChainApp/ucd/messages/route.hpp>
#include <UChainApp/ucd/services/block_service.hpp>
#include <UChainApp/ucd/services/heartbeat_service.hpp>
#include <UChainApp/ucd/services/query_service.hpp>
#include <UChainApp/ucd/services/tx_service.hpp>
#include <UChainApp/ucd/utility/address_key.hpp>
#include <UChainApp/ucd/utility/authenticator.hpp>
#include <UChainApp/ucd/utility/fetch_helpers.hpp>
#include <UChainApp/ucd/workers/notification_worker.hpp>
#include <UChainApp/ucd/workers/query_worker.hpp>

#endif
