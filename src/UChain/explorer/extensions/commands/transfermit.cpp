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
#include <UChain/explorer/extensions/commands/transfermit.hpp>
#include <UChain/explorer/extensions/command_extension_func.hpp>
#include <UChain/explorer/extensions/command_assistant.hpp>
#include <UChain/explorer/extensions/exception.hpp>
#include <UChain/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result transfermit::invoke (Json::Value& jv_output,
        libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check symbol
    check_card_symbol(argument_.symbol);

    // check to uid
    auto to_uid = argument_.to;
    auto to_address = get_address_from_uid(to_uid, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw toaddress_invalid_exception("Invalid uid parameter! " + to_uid);
    }

    // get identifiable token
    auto mits = blockchain.get_account_cards(auth_.name, argument_.symbol);
    if (mits->size() == 0) {
        throw token_lack_exception("Not enough token '" + argument_.symbol +  "'");
    }

    auto& mit = *(mits->begin());
    std::string from_address(mit.get_address());
    bool is_multisig_address = blockchain.is_script_address(from_address);

    account_multisig acc_multisig;
    if (is_multisig_address) {
        auto multisig_vec = acc->get_multisig(from_address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception("From address multisig record not found.");
        }

        acc_multisig = *(multisig_vec->begin());
    }

    // receiver
    std::vector<receiver_record> receiver{
        {
            to_address, argument_.symbol, 0, 0, 0,
            utxo_attach_type::token_card_transfer, uout("", to_uid)
        }
    };

    auto helper = transferring_card(
                      *this, blockchain,
                      std::move(auth_.name), std::move(auth_.auth),
                      is_multisig_address ? std::move(from_address) : "",
                      std::move(argument_.symbol),
                      std::move(receiver), argument_.fee,
                      std::move(acc_multisig));

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    if (is_multisig_address) {
        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx, false, true);
    }
    else {
        jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

