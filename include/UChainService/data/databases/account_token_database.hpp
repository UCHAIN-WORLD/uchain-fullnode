/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS) 
 *
 * This file is part of ucd.
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
#ifndef UC_DATABASE_ACCOUNT_TOKEN_DATABASE_HPP
#define UC_DATABASE_ACCOUNT_TOKEN_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/primitives/record_multimap.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <UChainService/txs/asset_data.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

struct BCD_API account_token_statinfo
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
/// which returns several rows giving the account_token for that address.
class BCD_API account_token_database
{
public:
    /// Construct the database.
    account_token_database(const boost::filesystem::path& lookup_filename,
        const boost::filesystem::path& rows_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~account_token_database();

    /// Initialize a new account_token database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    void store(const short_hash& key, const token_detail& account_token);

    void delete_last_row(const short_hash& key);

    token_detail::list get(const short_hash& key) const;

    std::shared_ptr<token_detail> get(const short_hash& key, const std::string& address) const;

    /// get tokens whose status is not issued and stored in local database (not in blockchain)
    std::shared_ptr<business_address_token::list> get_unissued_tokens(const short_hash& key) const;

    /// Synchonise with disk.
    void sync();

    /// Return statistical info about the database.
    account_token_statinfo statinfo() const;

private:
    typedef record_hash_table<short_hash> record_map;
    typedef record_multimap<short_hash> record_multiple_map;

    /// Hash table used for start index lookup for linked list by address hash.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;

    /// List of account_token rows.
    memory_map rows_file_;
    record_manager rows_manager_;
    record_list rows_list_;
    record_multiple_map rows_multimap_;
};

} // namespace database
} // namespace libbitcoin

#endif


