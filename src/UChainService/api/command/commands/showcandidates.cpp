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
#include <UChainService/api/command/commands/showcandidates.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/base_helper.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ showcandidates *************************/

console_result showcandidates::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    if (auth_.name.empty()) {
        // no wallet -- list whole tokens in blockchain
        auto sh_vec = blockchain.get_registered_candidates();
        if (nullptr != sh_vec) {
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem : *sh_vec) {
                if(elem.candidate.get_status() == CANDIDATE_STATUS_TRANSFER \
                   || elem.candidate.get_status() == CANDIDATE_STATUS_REGISTER)
                {
                    //elem.candidate.set_status(CANDIDATE_STATUS_REGISTER);
                    Json::Value token_data = json_helper.prop_list(elem);
                    json_value.append(token_data);
                }
                
            }
        }
    }
    else {
        blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

        // list tokens owned by wallet
        auto sh_vec = blockchain.get_wallet_candidates(auth_.name);
        if (nullptr != sh_vec) {
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem : *sh_vec) {
                // update content if it's transfered from others
                if (!elem.is_register_status()) {
                    auto token = blockchain.get_registered_candidate(elem.get_symbol());
                    if (nullptr != token) {
                        elem.set_content(token->candidate.get_content());
                    }
                }

                Json::Value token_data = json_helper.prop_list(elem, true);
                json_value.append(token_data);
            }
        }
    }

    /*if (get_api_version() <= 2) {
        jv_output["candidates"] = json_value;
    }
    else {*/
        if(json_value.isNull())
            json_value.resize(0);  
            
        jv_output = json_value;
    //}

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
