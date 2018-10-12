///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2018 libbitcoin-node developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UC_NODE_HPP
#define UC_NODE_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <UChain/blockchain.hpp>
#include <UChain/network.hpp>
#include <UChain/node/configuration.hpp>
#include <UChain/node/define.hpp>
#include <UChain/node/p2p_node.hpp>
#include <UChain/node/parser.hpp>
#include <UChain/node/settings.hpp>
#include <UChain/node/version.hpp>
#include <UChain/node/protocols/protocol_block_in.hpp>
#include <UChain/node/protocols/protocol_block_out.hpp>
#include <UChain/node/protocols/protocol_block_sync.hpp>
#include <UChain/node/protocols/protocol_header_sync.hpp>
#include <UChain/node/protocols/protocol_transaction_in.hpp>
#include <UChain/node/protocols/protocol_transaction_out.hpp>
#include <UChain/node/protocols/protocol_version_quiet.hpp>
#include <UChain/node/sessions/session_block_sync.hpp>
#include <UChain/node/sessions/session_header_sync.hpp>
#include <UChain/node/sessions/session_inbound.hpp>
#include <UChain/node/sessions/session_manual.hpp>
#include <UChain/node/sessions/session_outbound.hpp>
#include <UChain/node/utility/header_queue.hpp>
#include <UChain/node/utility/performance.hpp>
#include <UChain/node/utility/reservation.hpp>
#include <UChain/node/utility/reservations.hpp>

#endif
