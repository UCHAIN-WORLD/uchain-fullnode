/**
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
#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/result/transaction_result.hpp>
#include <UChain/database/primitives/slab_hash_table.hpp>
#include <UChain/database/primitives/slab_manager.hpp>

namespace libbitcoin
{
namespace database
{

/// This enables lookups of transactions by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This is so we can quickly reconstruct blocks given a list of tx indexes
/// belonging to that block. These are stored with the block.
class BCD_API blockchain_token_cert_database
{
  public:
    /// Construct the database.
    blockchain_token_cert_database(const boost::filesystem::path &map_filename,
                                   std::shared_ptr<shared_mutex> mutex = nullptr);

    /// Close the database (all threads must first be stopped).
    ~blockchain_token_cert_database();

    /// Initialize a new transaction database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    std::shared_ptr<token_cert> get(const hash_digest &hash) const;

    /// Get all token certs
    std::shared_ptr<std::vector<token_cert>> get_blockchain_token_certs() const;

    void store(const token_cert &sp_cert);

    /// Delete a transaction from database.
    void remove(const hash_digest &hash);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

  private:
    typedef slab_hash_table<hash_digest> slab_map;

    // Hash table used for looking up txs by hash.
    memory_map lookup_file_;
    slab_hash_table_header lookup_header_;
    slab_manager lookup_manager_;
    slab_map lookup_map_;
};

} // namespace database
} // namespace libbitcoin
