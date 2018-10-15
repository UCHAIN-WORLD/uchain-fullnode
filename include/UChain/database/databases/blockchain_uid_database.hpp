/**
 * Copyright (c) 2011-2018 libbitcoin developers 
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
#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/result/transaction_result.hpp>
#include <UChain/database/primitives/slab_hash_table.hpp>
#include <UChain/database/primitives/slab_manager.hpp>
#include <UChainService/txs/uid/blockchain_uid.hpp>

namespace libbitcoin {
namespace database {

/// This enables lookups of transactions by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This is so we can quickly reconstruct blocks given a list of tx indexes
/// belonging to that block. These are stored with the block.
class BCD_API blockchain_uid_database
{
public:
    /// Construct the database.
    blockchain_uid_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~blockchain_uid_database();

    /// Initialize a new transaction database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    std::shared_ptr<blockchain_uid> get(const hash_digest& hash) const;

    ///
    std::shared_ptr<std::vector<blockchain_uid> > get_history_uids(const hash_digest& hash) const;
    ///
    std::shared_ptr<std::vector<blockchain_uid> > get_blockchain_uids() const;

    /// 
    std::shared_ptr<blockchain_uid> get_register_history(const std::string & uid_symbol) const;
    ///
    uint64_t get_register_height(const std::string & uid_symbol) const;

    std::shared_ptr<std::vector<blockchain_uid> > getuids_from_address_history(
        const std::string &address, const uint64_t& fromheight = 0
        ,const uint64_t & toheight = max_uint64 ) const;

    void store(const hash_digest& hash, const blockchain_uid& sp_detail);

    /// Delete a transaction from database.
    void remove(const hash_digest& hash);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

    //pop back uid_detail
    std::shared_ptr<blockchain_uid> pop_uid_transfer(const hash_digest &hash);
protected:
    /// update address status(current or old), default old
     std::shared_ptr<blockchain_uid> update_address_status(const hash_digest& hash,uint32_t status = blockchain_uid::address_history);
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


