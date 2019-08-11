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
#ifndef UC_DATABASE_WALLET_ADDRESS_DATABASE_HPP
#define UC_DATABASE_WALLET_ADDRESS_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/primitives/record_multimap.hpp>
#include <UChainService/txs/wallet/wallet_address.hpp>
//#include <UChain/database/result/wallet_address_result.hpp>  // todo -- remove later
using namespace libbitcoin::chain;

namespace libbitcoin
{
namespace database
{

struct BCD_API wallet_address_statinfo
{
    /// Number of buckets used in the hashtable.
    /// load factor = addrs / buckets
    const size_t buckets;

    /// Total number of unique addresses in the database.
    const size_t addrs;

    /// Total number of rows across all addresses.
    const size_t rows;
};

/// This is a multimap where the key is the Bitcoin address hash,
/// which returns several rows giving the wallet_address for that address.
class BCD_API wallet_address_database
{
  public:
    /// Construct the database.
    wallet_address_database(const boost::filesystem::path &lookup_filename,
                            const boost::filesystem::path &rows_filename,
                            std::shared_ptr<shared_mutex> mutex = nullptr);

    /// Close the database (all threads must first be stopped).
    ~wallet_address_database();

    /// Initialize a new wallet_address database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// store wallet address into database
    void store(const short_hash &key, const wallet_address &wallet_address);

    /// get wallet address vector by key
    wallet_address::list get(const short_hash &key) const;

    /// get wallet address according by key and address
    std::shared_ptr<wallet_address> get(const short_hash &key, const std::string &address) const;

    /// Delete the last row that was added to key.
    void delete_last_row(const short_hash &key);

    void safe_store(const short_hash &key, const wallet_address &address);

    /// Synchonise with disk.
    void sync();

    /// Return statistical info about the database.
    wallet_address_statinfo statinfo() const;

  private:
    typedef record_hash_table<short_hash> record_map;
    typedef record_multimap<short_hash> record_multiple_map;

    /// Hash table used for start index lookup for linked list by address hash.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;

    /// List of wallet_address rows.
    memory_map rows_file_;
    record_manager rows_manager_;
    record_list rows_list_;
    record_multiple_map rows_multimap_;
};

} // namespace database
} // namespace libbitcoin

#endif
