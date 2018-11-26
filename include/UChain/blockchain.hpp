///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2018 libbitcoin-blockchain developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UC_BLOCKCHAIN_HPP
#define UC_BLOCKCHAIN_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <UChain/database.hpp>

#ifdef WITH_CONSENSUS
#include <UChainService/consensus.hpp>
#endif

#include <UChain/blockchain/block.hpp>
#include <UChain/blockchain/block_chain.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChain/blockchain/block_detail.hpp>
#include <UChain/blockchain/block_fetcher.hpp>
#include <UChain/blockchain/define.hpp>
#include <UChain/blockchain/organizer.hpp>
#include <UChain/blockchain/orphan_pool.hpp>
#include <UChain/blockchain/settings.hpp>
#include <UChain/blockchain/simple_chain.hpp>
#include <UChain/blockchain/transaction_pool.hpp>
#include <UChain/blockchain/transaction_pool_index.hpp>
#include <UChain/blockchain/validate_block.hpp>
#include <UChain/blockchain/validate_block_impl.hpp>
#include <UChain/blockchain/validate_transaction.hpp>
#include <UChain/blockchain/version.hpp>

#endif
