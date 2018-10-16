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


#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/deletetoken.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;
/************************ deletetoken *************************/

console_result deletetoken::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);

    if (blockchain.get_issued_token(option_.symbol))
        throw token_issued_not_delete{"Cannot delete token " + option_.symbol + " which has been issued."};

    std::promise<code> p;
    std::vector<libbitcoin::blockchain::transaction_pool::transaction_ptr> txs;
    blockchain.pool().fetch([&txs, &p](const code& ec,
        const std::vector<libbitcoin::blockchain::transaction_pool::transaction_ptr>& tx)
    {
        if (!ec) {
            txs = tx;
        }

        p.set_value(ec);
    });
    p.get_future().get();

    for(auto& tx : txs) {
        for(auto& output : tx->outputs) {
            if (output.is_token_issue() && output.get_token_symbol() == option_.symbol) {
                throw token_issued_not_delete{"Cannot delete token " + option_.symbol + " which has been issued."};
            }
        }
    }

    std::vector<business_address_token> tokens = *blockchain.get_account_unissued_tokens(auth_.name);
    bool found = false;
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->detail.get_symbol() == option_.symbol) {
            if (blockchain.delete_account_token(auth_.name) == console_result::failure) {
                throw token_delete_fail{"token " + option_.symbol + " delete fail."};
            }

            tokens.erase(it);
            for (auto token : tokens) {
                blockchain.store_account_token(token.detail, auth_.name);
            }

            found = true;
            break;
        }
    }

    if (!found) {
        throw token_notfound_exception{"token " + option_.symbol + " does not existed or do not belong to " + auth_.name + "."};
    }

    if (get_api_version() <= 2) {
        jv_output["symbol"] = option_.symbol;
        jv_output["operate"] = "delete";
        jv_output["result"] = "success";
    }
    else {
        jv_output["symbol"] = option_.symbol;
        jv_output["status"]= "deleted successfully";
    }

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin
