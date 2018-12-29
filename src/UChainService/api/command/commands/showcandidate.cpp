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
#include <UChainService/api/command/commands/showcandidate.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

/************************ showcandidate *************************/

console_result showcandidate::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    if (!argument_.symbol.empty()) {
        // check symbol
        check_candidate_symbol(argument_.symbol);
    }

    if (option_.show_current) {
        if (argument_.symbol.empty()) {
            throw argument_legality_exception("candidate symbol not privided while displaying the current status of candidate!");
        }
    }


    if (argument_.symbol.empty()) {
        throw argument_legality_exception("candidate symbol not privided while tracing history!");
    }

    // page limit & page index paramenter check
    if (option_.index < 1) {
        throw argument_legality_exception{"page index parameter cannot be zero"};
    }

    if (option_.limit < 1) {
        throw argument_legality_exception{"page record limit parameter cannot be zero"};
    }

    if (option_.limit > 100) {
        throw argument_legality_exception{"page record limit cannot be bigger than 100."};
    }
  

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    bool is_list = true;
    if (argument_.symbol.empty()) {
        auto sh_vec = blockchain.get_registered_candidates();
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem : *sh_vec) {
            json_value.append(elem.candidate.get_symbol());
        }

        if (get_api_version() <=2 ) {
            jv_output["candidates"] = json_value;
        }
        else {
            jv_output = json_value;
        }
    }
    else {
        if (option_.show_current) {
            auto sh_vec = blockchain.get_candidate_history(argument_.symbol, 1, 1);
            if (nullptr != sh_vec && sh_vec->size() > 0) {
                auto last_iter = --(sh_vec->end());
                auto& candidate_info = *last_iter;
                auto reg_candidate = blockchain.get_registered_candidate(argument_.symbol);
                if (nullptr != reg_candidate) {
                    candidate_info.candidate.set_content(reg_candidate->candidate.get_content());
                }

                json_value = json_helper.prop_list(candidate_info, true);
            }
        }
        else {
            auto sh_vec = blockchain.get_candidate_history(argument_.symbol, option_.limit, option_.index);
            for (auto& elem : *sh_vec) {
                Json::Value token_data = json_helper.prop_list(elem);
                json_value.append(token_data);
            }

            if (get_api_version() <=2 ) {
                jv_output["candidates"] = json_value;
            }
            else {
                if(json_value.isNull())
                    json_value.resize(0);  

                jv_output = json_value;
            }
        }

        jv_output = json_value;
     
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

