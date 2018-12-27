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


#include <jsoncpp/json/json.h>
#include <UChain/client.hpp>
#include <UChain/explorer/callback_state.hpp>
#include <UChainService/api/command/node_method_wrapper.hpp>
#include <UChainService/api/command/commands/showblockheaders.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/display.hpp>
#include <UChain/explorer/utility.hpp>
#include <UChain/explorer/define.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::client;
using namespace bc::explorer::config;

/************************ showblockheaders *************************/

console_result showblockheaders::invoke(Json::Value& jv_output,
                                      libbitcoin::server::server_node& node)
{

    using namespace libbitcoin::config; // for hash256
    auto& blockchain = node.chain_impl();
    administrator_required_checker(node, auth_.name, auth_.auth);
    //blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // height check
    if (option_.height.first()
            && option_.height.second()
            && (option_.height.first() >= option_.height.second())) {
        throw block_height_exception{"invalid height option!"};
    }
    
    uint64_t end;
    if(blockchain.get_last_height(end))
    {
        if(end > option_.height.second())
            end = option_.height.second();
    }
    if(end - option_.height.first() > 1000)
    {
        throw block_height_exception{"Cannot get block headers much than 10000!"};
    }
    
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection)) {
        throw connection_exception{"Could not connect to ucd port 9921."};
    }

    encoding json_format{"json"};
    std::ostringstream output;
    std::vector<Json::Value> headers;
    callback_state state(output, output, json_format);

    auto on_done = [this, &jv_output, &end](const chain::header & header)
    {
        
        auto&& jheader = config::json_helper(get_api_version()).prop_tree(header);
        if (!jheader.isObject() || !jheader["hash"].isString()) {
            throw block_hash_get_exception{"getbestblockhash parser exception."};
        }
        jv_output.append(jheader);  
    };

    auto on_error = [&state](const code & error)
    {
        state.succeeded(error);
    };

    // Height is ignored if both are specified.
    // Use the null_hash as sentinel to determine whether to use height or hash.

    for(size_t num=option_.height.first(); num<= end; num++)
    {
        client.blockchain_fetch_block_header(on_error, on_done, num);
        client.wait();
    }

    return state.get_result();
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

