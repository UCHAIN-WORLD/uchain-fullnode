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
#ifndef UC_DATABASE_TOKEN_DATABASE_HPP
#define UC_DATABASE_TOKEN_DATABASE_HPP
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/result/token_result.hpp>
#include <UChain/database/primitives/slab_hash_table.hpp>
#include <UChain/database/primitives/slab_manager.hpp>
#include <UChain/database/databases/base_database.hpp>
#include <UChainService/txs/token/token_detail.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

/// This enables lookups of tokens by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This database is used to store token issued from blockchain(not store local unissued tokens)
class BCD_API token_database: public base_database
{
public:
    /// Construct the database.
    token_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~token_database();

    /// get token info by symbol hash
    token_result get_token_result(const hash_digest& hash) const;
    /// get all tokens in the blockchain
    std::shared_ptr<std::vector<token_detail>> get_token_details() const;
    /// Store a token in the database. Returns a unique index
    /// which can be used to reference the token.
    void store(const hash_digest& hash, const token_detail& sp_detail);
};

} // namespace database
} // namespace libbitcoin

#endif
