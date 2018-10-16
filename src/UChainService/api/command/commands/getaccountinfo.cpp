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
#include <UChainService/api/command/commands/getaccountinfo.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaccountinfo *************************/

console_result getaccountinfo::invoke(Json::Value& jv_output,
                                  libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    //auto&& mnemonic = acc->get_mnemonic(auth_.auth);
    std::string&& mnemonic = blockchain.is_account_lastwd_valid(*acc, auth_.auth, argument_.last_word);

    auto& root = jv_output;

    if (get_api_version() == 1) {
        root["name"] = acc->get_name();
        root["mnemonic-key"] = mnemonic;
        root["address-count"] += acc->get_hd_index() + 1;
        root["user-status"] += (uint8_t)account_status::normal;
    }
    else if (get_api_version() == 2) {
        root["name"] = acc->get_name();
        root["mnemonic-key"] = mnemonic;
        root["address-count"] = acc->get_hd_index() + 1;
        root["user-status"] = (uint8_t)account_status::normal;
    }
    else {
        root["name"] = acc->get_name();
        root["mnemonic"] = mnemonic;
        root["address_count"] = acc->get_hd_index() + 1;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

