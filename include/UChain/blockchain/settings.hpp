/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
 *
 * This file is part of UChain-blockchain.
 *
 * UChain-blockchain is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_BLOCKCHAIN_SETTINGS_HPP
#define UC_BLOCKCHAIN_SETTINGS_HPP

#include <cstdint>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

/// Common database configuration settings, properties not thread safe.
class BCB_API settings
{
public:
    settings();
    settings(bc::settings context);

    /// Properties.
    uint32_t block_pool_capacity;
    uint32_t transaction_pool_capacity;
    bool transaction_pool_consistency;
    bool use_testnet_rules;
    config::checkpoint::list checkpoints;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
