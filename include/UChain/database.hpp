///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-database developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UC_DATABASE_HPP
#define UC_DATABASE_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <UChain/bitcoin.hpp>
#include <UChain/database/data_base.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/settings.hpp>
#include <UChain/database/version.hpp>
#include <UChain/database/databases/block_database.hpp>
#include <UChain/database/databases/history_database.hpp>
#include <UChain/database/databases/spend_database.hpp>
#include <UChain/database/databases/stealth_database.hpp>
#include <UChain/database/databases/transaction_database.hpp>
#include <UChain/database/memory/accessor.hpp>
#include <UChain/database/memory/allocator.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/primitives/hash_table_header.hpp>
#include <UChain/database/primitives/record_hash_table.hpp>
#include <UChain/database/primitives/record_list.hpp>
#include <UChain/database/primitives/record_manager.hpp>
#include <UChain/database/primitives/record_multimap.hpp>
#include <UChain/database/primitives/record_multimap_iterable.hpp>
#include <UChain/database/primitives/record_multimap_iterator.hpp>
#include <UChain/database/primitives/slab_hash_table.hpp>
#include <UChain/database/primitives/slab_manager.hpp>
#include <UChain/database/result/block_result.hpp>
#include <UChain/database/result/transaction_result.hpp>

#endif
