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
#include <UChainService/api/command/commands/showuids.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/base_helper.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ showuids *************************/

console_result showuids::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    // page limit & page index paramenter check
    if (argument_.index <= 0) {
        throw argument_legality_exception{"page index parameter cannot be zero"};
    }
    if (argument_.limit <= 0) {
        throw argument_legality_exception{"page record limit parameter cannot be zero"};
    }
    if (argument_.limit > 100) {
        throw argument_legality_exception{"page record limit cannot be bigger than 100."};
    }

    auto& blockchain = node.chain_impl();
    std::shared_ptr<uid_detail::list> sh_vec;
    if (auth_.name.empty()) {
        // no account -- list all uids in blockchain
        sh_vec = blockchain.get_registered_uids();
    }
    else {
        // list uids owned by the account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        sh_vec = blockchain.get_account_uids(auth_.name);
    }

    uint64_t limit = argument_.limit;
    uint64_t index = argument_.index;

    std::vector<uid_detail> result;
    uint64_t total_count = sh_vec-> size();
    uint64_t total_page = 0;
    if (total_count > 0) {
        std::sort(sh_vec->begin(), sh_vec->end());

        uint64_t start = 0, end = 0, tx_count = 0;
        if (index && limit) {
            total_page = (total_count % limit) ? (total_count / limit + 1) : (total_count / limit);
            index = index > total_page ? total_page : index;
            start = (index - 1) * limit;
            end = index * limit;
            tx_count = end >= total_count ? (total_count - start) : limit ;
        }
        else if (!index && !limit) { // all tx records
            start = 0;
            tx_count = total_count;
            index = 1;
            total_page = 1;
        }
        else {
            throw argument_legality_exception{"invalid limit or index parameter"};
        }

        if (start < total_count && tx_count > 0) {
            result.resize(tx_count);
            std::copy(sh_vec->begin() + start, sh_vec->begin() + start + tx_count, result.begin());
        }
    }

    Json::Value uids;
    // add blockchain uids
    for (auto& elem: result) {
        Json::Value uid_data;
        uid_data["symbol"] = elem.get_symbol();
        uid_data["address"] = elem.get_address();
        uid_data["status"] = "registered";
        uids.append(uid_data);
    }

    // output
    if (uids.isNull()) {
        uids.resize(0);
    }

    jv_output["total_count"] = total_count;
    jv_output["total_page"] = total_page;
    jv_output["current_page"] = index;
    jv_output["uids"] = uids;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
