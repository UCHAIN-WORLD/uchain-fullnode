/**
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS) 
 *
 * This file is part of uc-node.
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
#ifndef UC_DATABASE_ACCOUNT_DATABASE_HPP
#define UC_DATABASE_ACCOUNT_DATABASE_HPP
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/result/account_result.hpp>
#include <UChain/database/primitives/slab_hash_table.hpp>
#include <UChain/database/primitives/slab_manager.hpp>
#include <UChain/database/databases/base_database.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

/// This enables lookups of accounts by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This is so we can quickly reconstruct blocks given a list of tx indexes
/// belonging to that block. These are stored with the block.
class BCD_API account_database: public base_database
{
public:
    /// Construct the database.
    account_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~account_database();

    void set_admin(const std::string& name, const std::string& passwd);
    /// get account info by symbol hash
    account_result get_account_result(const hash_digest& hash) const;
    std::shared_ptr<std::vector<account>> get_accounts() const;

    /// Store a account in the database. Returns a unique index
    /// which can be used to reference the account.
    void store(const account& account);
};

} // namespace database
} // namespace libbitcoin

#endif
