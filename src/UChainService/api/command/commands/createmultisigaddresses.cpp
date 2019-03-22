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
#include <UChainService/api/command/commands/createmultisigaddress.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

console_result createmultisigaddress::invoke(
    Json::Value &jv_output,
    libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();

    // check auth
    auto wallet = blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    auto &pubkey_vec = option_.public_keys;
    if (pubkey_vec.empty())
    {
        throw multisig_cosigne_exception{"multisig cosigner public key needed."};
    }

    std::set<std::string> unique_keys(pubkey_vec.begin(), pubkey_vec.end());
    if (unique_keys.size() != pubkey_vec.size())
    {
        throw multisig_cosigne_exception{"multisig cosigner public key has duplicated items."};
    }

    // check m & n
    if (option_.m < 1)
    {
        throw signature_amount_exception{"signature number less than 1."};
    }
    if (option_.n < 1 || option_.n > 20)
    {
        throw pubkey_amount_exception{
            "public key number " + std::to_string(option_.n) + " less than 1 or bigger than 20."};
    }
    if (option_.m > option_.n)
    {
        throw signature_amount_exception{
            "signature number " + std::to_string(option_.m) + " is bigger than public key number " + std::to_string(option_.n)};
    }

    // check self public key
    auto self_pubkey = option_.self_publickey;
    if (self_pubkey.empty())
    {
        throw pubkey_notfound_exception{"self pubkey key not found!"};
    }

    // if self public key not in public keys then add it.
    auto iter = std::find(pubkey_vec.begin(), pubkey_vec.end(), self_pubkey);
    if (iter == pubkey_vec.end())
    {
        pubkey_vec.push_back(self_pubkey);
    }

    // check public key size
    if (option_.n != pubkey_vec.size())
    {
        throw pubkey_amount_exception{"public key number does not match with n."};
    }

    // get private key according public key
    auto pvaddr = blockchain.get_wallet_addresses(auth_.name);
    if (!pvaddr)
    {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    std::string self_prvkey;
    auto found = false;
    for (auto &each : *pvaddr)
    {
        self_prvkey = each.get_prv_key(auth_.auth);
        auto &&target_pub_key = ec_to_xxx_impl("ec-to-public", self_prvkey);
        if (target_pub_key == self_pubkey)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw pubkey_dismatch_exception{self_pubkey + " does not belongs to this wallet"};
    }

    // generate multisig wallet
    wallet_multisig acc_multisig;
    acc_multisig.set_hd_index(0);
    acc_multisig.set_m(option_.m);
    acc_multisig.set_n(option_.n);
    acc_multisig.set_pub_key(self_pubkey);
    acc_multisig.set_cosigner_pubkeys(std::move(pubkey_vec));
    acc_multisig.set_description(option_.description);

    // check same multisig wallet not exists
    if (wallet->is_multisig_exist(acc_multisig))
        throw multisig_exist_exception{"multisig already exists."};

    // update index
    acc_multisig.set_index(wallet->get_multisig_vec().size() + 1);

    // change wallet type
    wallet->set_type(wallet_type::multisignature);

    // create wallet address
    auto wallet_address = std::make_shared<bc::chain::wallet_address>();
    wallet_address->set_name(auth_.name);
    wallet_address->set_prv_key(self_prvkey, auth_.auth);

    // create payment script and address
    auto multisig_script = acc_multisig.get_multisig_script();
    chain::script payment_script;
    payment_script.from_string(multisig_script);
    if (script_pattern::pay_multisig != payment_script.pattern())
        throw multisig_script_exception{std::string("invalid multisig script : ") + multisig_script};

    payment_address address(payment_script, payment_address::mainnet_p2sh);
    auto hash_address = address.encoded();

    // update wallet and multisig wallet
    // wallet_address->set_status(1); // 1 -- enable address
    wallet_address->set_address(hash_address);
    wallet_address->set_status(wallet_address_status::multisig_addr);

    acc_multisig.set_address(hash_address);
    wallet->set_multisig(acc_multisig);

    // store them
    blockchain.store_wallet(wallet);
    blockchain.store_wallet_address(wallet_address);

    // output json
    jv_output = config::json_helper(get_api_version()).prop_list(acc_multisig);
    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
