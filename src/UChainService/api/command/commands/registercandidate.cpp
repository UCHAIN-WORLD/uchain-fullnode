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
#include <UChainService/api/command/commands/registercandidate.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>
#include <UChain/bitcoin/config/authority.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

void registercandidate::check_symbol_content(const std::string& symbol, const std::string& content)
{
    // check symbol
    if (symbol.size() == 0) {
        throw token_symbol_length_exception{"Symbol can not be empty."};
    }

    // reserve 4 bytes
    if (symbol.size() > (TOKEN_CANDIDATE_SYMBOL_FIX_SIZE - 4)) {
        throw token_symbol_length_exception{"Symbol length must be less than "
            + std::to_string(TOKEN_CANDIDATE_SYMBOL_FIX_SIZE - 4) + ". " + symbol};
    }

    // check symbol
    check_candidate_symbol(symbol, true);

    // check content
    if (content.size() > TOKEN_CANDIDATE_CONTENT_FIX_SIZE) {
        throw argument_size_invalid_exception(
            "Content length must be less than "
            + std::to_string(TOKEN_CANDIDATE_CONTENT_FIX_SIZE) + ". " + content);
    }
}

console_result registercandidate::invoke (Json::Value& jv_output,
        libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    std::map<std::string, std::string> candidate_map;

    bool use_unified_content = false;
    // check single symbol and content
    if (argument_.symbol.size() > 0) {
        check_symbol_content(argument_.symbol, option_.content);

        // check symbol not registered
        if (blockchain.get_registered_candidate(argument_.symbol)) {
            throw token_symbol_existed_exception{"candidate already exists in blockchain. " + argument_.symbol};
        }

        candidate_map[argument_.symbol] = option_.content;
    }
    else {
        if (option_.content.size() > 0) {
            // check content
            if (option_.content.size() > TOKEN_CANDIDATE_CONTENT_FIX_SIZE) {
                throw argument_size_invalid_exception(
                    "Content length must be less than "
                    + std::to_string(TOKEN_CANDIDATE_CONTENT_FIX_SIZE) + ". " + option_.content);
            }

            use_unified_content = true;
        }
    }

    // check multi symbol and content
    /*for (const auto& candidate : option_.multicandidates) {
        std::string symbol, content;
        auto pos = candidate.find_first_of(":");
        if (pos == std::string::npos) {
            symbol = candidate;

            if (use_unified_content) {
                content = option_.content;
            }
            else {
                content = "";
            }
        }
        else {
            symbol = candidate.substr(0, pos);
            content = candidate.substr(pos + 1);
        }

        check_symbol_content(symbol, content);

        if (candidate_map.find(symbol) != candidate_map.end()) {
            throw token_symbol_existed_exception{"Duplicate symbol: " + symbol};
        }

        // check symbol not registered
        if (blockchain.get_registered_candidate(symbol)) {
            throw token_symbol_existed_exception{"candidate already exists in blockchain. " + symbol};
        }

        candidate_map[symbol] = content;
    }*/

    if (candidate_map.empty()) {
        throw argument_legality_exception{"No symbol provided."};
    }

    try {
        const auto authority = libbitcoin::config::authority(argument_.symbol);
        if (!authority.to_network_address().is_routable()) {
            throw address_invalid_exception{"NODEADDRESS is not routable! "};
        }
    }
    catch (...)
    {
        throw address_invalid_exception{"NODEADDRESS is not valid! "};
    }   
    

    // check to uid
    auto to_uid = argument_.to;
    auto to_address = get_address_from_uid(to_uid, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw address_invalid_exception{"invalid uid parameter! " + to_uid};
    }
    if (!blockchain.get_wallet_address(auth_.name, to_address)) {
        throw address_dismatch_wallet_exception{"target uid does not match wallet. " + to_uid};
    }

    // receiver
    std::vector<receiver_record> receiver;
    for (auto& pair : candidate_map) {
        receiver.push_back(
            {
                to_address, pair.first, 0, 0, 0,
                utxo_attach_type::token_candidate, asset(to_uid, "")
            }
        );
    }

    receiver.push_back({to_address, "", bc::min_lock_to_issue_candidate, 0, utxo_attach_type::deposit, asset{"", to_uid}});

    auto helper = registering_candidate(
                      *this, blockchain,
                      std::move(auth_.name), std::move(auth_.auth),
                      std::move(to_address),
                      "", std::move(candidate_map),
                      std::move(receiver), argument_.fee);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

