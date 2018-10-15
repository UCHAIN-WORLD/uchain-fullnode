/**
 * Copyright (c) 2018-2020 uc developers
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
#include <UChain/explorer/extensions/commands/listmits.hpp>
#include <UChain/explorer/extensions/command_extension_func.hpp>
#include <UChain/explorer/extensions/command_assistant.hpp>
#include <UChain/explorer/extensions/base_helper.hpp>
#include <UChain/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listmits *************************/

console_result listmits::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    if (auth_.name.empty()) {
        // no account -- list whole tokens in blockchain
        auto sh_vec = blockchain.get_registered_mits();
        if (nullptr != sh_vec) {
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem : *sh_vec) {
                Json::Value token_data = json_helper.prop_list(elem);
                json_value.append(token_data);
            }
        }
    }
    else {
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

        // list tokens owned by account
        auto sh_vec = blockchain.get_account_mits(auth_.name);
        if (nullptr != sh_vec) {
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem : *sh_vec) {
                // update content if it's transfered from others
                if (!elem.is_register_status()) {
                    auto token = blockchain.get_registered_mit(elem.get_symbol());
                    if (nullptr != token) {
                        elem.set_content(token->mit.get_content());
                    }
                }

                Json::Value token_data = json_helper.prop_list(elem, true);
                json_value.append(token_data);
            }
        }
    }

    if (get_api_version() <= 2) {
        jv_output["mits"] = json_value;
    }
    else {
        if(json_value.isNull())
            json_value.resize(0);  
            
        jv_output = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
