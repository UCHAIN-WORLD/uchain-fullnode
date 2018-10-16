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
#include <UChain/explorer/extensions/commands/uidchangeaddress.hpp>
#include <UChain/explorer/extensions/command_extension_func.hpp>
#include <UChain/explorer/extensions/command_assistant.hpp>
#include <UChain/explorer/extensions/exception.hpp>
#include <UChain/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result uidchangeaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check uid symbol
    auto uid = argument_.symbol;

    check_uid_symbol(uid);

    // check uid exsits
    auto uid_detail = blockchain.get_registered_uid(uid);
    if (!uid_detail) {
        throw uid_symbol_notfound_exception{"Did '" + uid + "' does not exist on the blockchain"};
    }

    auto from_address = uid_detail->get_address();

    // check uid is owned by the account
    if (!blockchain.get_account_address(auth_.name, from_address)) {
        throw uid_symbol_notowned_exception{
            "Did '" + uid + "' is not owned by " + auth_.name};
    }

    // check to address is valid
    if (!blockchain.is_valid_address(argument_.to))
        throw toaddress_invalid_exception{"Invalid target address parameter!"};

    // check to address is owned by the account
    if (!blockchain.get_account_address(auth_.name, argument_.to)) {
        throw address_dismatch_account_exception{"Target address is not owned by account. " + argument_.to};
    }

     // fail if address is already binded with uid in blockchain
    if (blockchain.is_address_registered_uid(argument_.to)) {
        throw uid_symbol_existed_exception{"Target address is already binded with some uid on the blockchain"};
    }

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, argument_.symbol, 0, 0, utxo_attach_type::uid_transfer, uout()}
    };

    auto toaddr = bc::wallet::payment_address(argument_.to);
    auto addr = bc::wallet::payment_address(from_address);

    if( toaddr.version() == bc::wallet::payment_address::mainnet_p2sh
    && addr.version() == bc::wallet::payment_address::mainnet_p2sh)
        throw uid_multisig_address_exception{"uid cannot modify multi-signature address to multi-signature address"};


    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh
    || toaddr.version() == bc::wallet::payment_address::mainnet_p2sh) // for multisig address
    {

        auto findmultisig = [&acc](account_multisig& acc_multisig, std::string address) {
            auto multisig_vec = acc->get_multisig(address);
            if (!multisig_vec || multisig_vec->empty())
                return false;

            acc_multisig = *(multisig_vec->begin());
            return true;
        };

        account_multisig acc_multisig;
        if (addr.version() == bc::wallet::payment_address::mainnet_p2sh && !findmultisig(acc_multisig, from_address))
            throw multisig_notfound_exception{"from address multisig record not found."};

        account_multisig acc_multisig_to;
        if (toaddr.version() == bc::wallet::payment_address::mainnet_p2sh && !findmultisig(acc_multisig_to, argument_.to))
            throw multisig_notfound_exception{"to address multisig record not found."};

        auto send_helper = sending_multisig_uid(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                                                 std::move(from_address),  std::move(argument_.to),
                                                 std::move(argument_.symbol), std::move(receiver), argument_.fee,
                                                 std::move(acc_multisig), std::move(acc_multisig_to));

        send_helper.exec();
        // json output
        auto && tx = send_helper.get_transaction();
        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx, false, true);
    }
    else
    {
        auto send_helper = sending_uid(*this, blockchain,
                                       std::move(auth_.name), std::move(auth_.auth),
                                       std::move(from_address), std::move(argument_.to),
                                       std::move(argument_.symbol), std::move(receiver), argument_.fee);

        send_helper.exec();

        // json output
        auto tx = send_helper.get_transaction();
        jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);
    }




    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

