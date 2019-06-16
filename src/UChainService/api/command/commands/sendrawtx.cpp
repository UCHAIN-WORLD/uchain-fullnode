/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
 *
 * UChain-explorer is free software: you can redistribute it and/or
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

#include <UChain/explorer/json_helper.hpp>
#include <UChainService/api/command/commands/sendrawtx.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

console_result sendrawtx::invoke(Json::Value &jv_output,
                                 libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    tx_type tx_ = argument_.transaction;

    uint64_t outputs_ucn_val = tx_.total_output_value();
    uint64_t inputs_ucn_val = 0;
    if (!blockchain.get_tx_inputs_ucn_value(tx_, inputs_ucn_val))
        throw tx_validate_exception{"get transaction inputs ucn value error!"};

    // check raw tx fee range
    if (inputs_ucn_val <= outputs_ucn_val)
        throw tx_validate_exception{"no enough transaction fee"};
    base_transfer_common::check_fee_in_valid_range(inputs_ucn_val - outputs_ucn_val);

    if (blockchain.validate_tx_engine(tx_))
        throw tx_validate_exception{"validate transaction failure"};

    if (blockchain.broadcast_transaction(tx_))
        throw tx_broadcast_exception{"broadcast transaction failure"};

    if (get_api_version() <= 2)
    {
        jv_output["hash"] = encode_hash(tx_.hash());
    }
    else
    {
        jv_output = encode_hash(tx_.hash());
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
