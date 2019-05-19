/**
 * Copyright (c) 2018-2020 UChain developers 
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
#ifndef UC_DATABASE_WALLET_RESULT_HPP
#define UC_DATABASE_WALLET_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/result/base_result.hpp>
#include <UChainService/txs/wallet/wallet.hpp>

using namespace libbitcoin::chain;
using namespace std;

namespace libbitcoin
{
namespace database
{

/// read wallet detail information from wallet database.
class BCD_API wallet_result : public base_result
{
  public:
    wallet_result(const memory_ptr slab);

    /// The wallet.
    //wallet get_wallet_detail() const;
    std::shared_ptr<libbitcoin::chain::wallet> get_wallet_detail() const;
};

} // namespace database
} // namespace libbitcoin

#endif
