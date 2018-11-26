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
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/showtx.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ showtx *************************/
/// extent fetch-tx command , add tx height in tx content
console_result showtx::invoke(Json::Value& jv_output,
                             libbitcoin::server::server_node& node)
{
    bc::chain::transaction tx;
    uint64_t tx_height = 0;
    auto& blockchain = node.chain_impl();
    auto exist = blockchain.get_transaction(argument_.hash, tx, tx_height);
    if (!exist) {
        throw tx_notfound_exception{"transaction does not exist!"};
    }

    auto json_helper = config::json_helper(get_api_version());
    if (option_.json) {
        if (get_api_version() == 1 && option_.is_fetch_tx) { // compatible for v1 fetch-tx
            jv_output = json_helper.prop_tree(tx, true);
        }
        else {
            jv_output = json_helper.prop_list(tx, tx_height, true);
        }
    }
    else {
        jv_output = json_helper.prop_tree(tx, false);
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

