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
#include <UChainService/api/command/commands/sendto.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result sendto::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    asset attach;
    std::string to_address = get_address(argument_.to, attach, false, blockchain);
    std::string change_address = get_address(option_.change, blockchain);

    if (argument_.amount <= 0) {
        throw argument_legality_exception("invalid amount parameter!");
    }

    //should own the address
    if (!change_address.empty() && !blockchain.get_wallet_address(auth_.name, change_address))
        throw wallet_authority_exception{"change address not belongs to you."};

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, "", argument_.amount, 0, utxo_attach_type::ucn, attach}
    };

    if (!option_.memo.empty()) {
        if ( option_.memo.size() >= 255) {
            throw argument_size_invalid_exception{"memo length out of bounds."};
        }

        receiver.push_back({to_address, "", 0, 0, utxo_attach_type::message,
            asset(0, 0, blockchain_message(option_.memo))});
    }

    auto send_helper = sending_ucn(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        "", std::move(receiver),
        std::move(change_address), option_.fee);

    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

