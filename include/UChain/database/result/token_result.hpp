/**
 * Copyright (c) 2011-2018 UChain developers 
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
#ifndef UC_DATABASE_TOKEN_RESULT_HPP
#define UC_DATABASE_TOKEN_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <UChain/bitcoin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/result/base_result.hpp>
#include <UChainService/txs/token/token_detail.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

/// read token detail information from token database.
class BCD_API token_result : public base_result
{
public:
    token_result(const memory_ptr slab);

    /// The token.
    std::shared_ptr<token_detail> get_token_detail() const;
};

} // namespace database
} // namespace libbitcoin

#endif
