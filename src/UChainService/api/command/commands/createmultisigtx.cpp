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
#include <UChainService/api/command/commands/createmultisigtx.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

using namespace bc::explorer::config;

console_result createmultisigtx::invoke(
    Json::Value &jv_output,
    libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    auto wallet = blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    // check from address
    if (!blockchain.is_valid_address(argument_.from))
    {
        throw fromaddress_invalid_exception{"invalid from address!"};
    }

    auto addr = bc::wallet::payment_address(argument_.from);
    if (addr.version() != bc::wallet::payment_address::mainnet_p2sh)
    {
        throw fromaddress_invalid_exception{"from address is not a script address."};
    }

    auto multisig_vec = wallet->get_multisig(argument_.from);
    if (!multisig_vec || multisig_vec->empty())
    {
        throw multisig_notfound_exception{"multisig of from address not found."};
    }

    wallet_multisig acc_multisig = *(multisig_vec->begin());

    // check to address
    if (!blockchain.is_valid_address(argument_.to))
    {
        throw toaddress_invalid_exception{"invalid to address!"};
    }

    // receiver
    std::vector<receiver_record> receiver;

    auto type = static_cast<utxo_attach_type>(option_.type);
    switch (type)
    {
    case utxo_attach_type::ucn:
    {
        receiver.push_back({argument_.to, "", argument_.amount, 0, type, asset()});
        break;
    }

    case utxo_attach_type::token_transfer:
    {
        blockchain.uppercase_symbol(option_.symbol);
        check_token_symbol(option_.symbol);
        if (option_.symbol == UC_VOTE_TOKEN_SYMBOL)
        {
            throw token_symbol_name_exception{"Cannot send 'VOTE' token in this way.Please use vote command."};
        }
        receiver.push_back({argument_.to, option_.symbol, 0, argument_.amount, type, asset()});
        break;
    }

    default:
    {
        throw argument_legality_exception{"invalid transaction type."};
    }
    break;
    }

    auto sp_send_helper = std::make_shared<sending_multisig_tx>(*this, blockchain,
                                                                std::move(auth_.name), std::move(auth_.auth),
                                                                std::move(argument_.from), std::move(receiver),
                                                                argument_.fee, acc_multisig, std::move(option_.symbol));

    sp_send_helper->exec();

    // output json
    auto &&tx = sp_send_helper->get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx, false, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
