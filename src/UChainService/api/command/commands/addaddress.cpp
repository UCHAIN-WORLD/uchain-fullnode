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


#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/addaddress.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ addaddress *************************/

console_result addaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (!option_.count || (option_.count & 0xfff00000)) {
        throw address_amount_exception("invalid address number parameter");
    }
    //operation
    if (option_.operation == "del")
    {
        blockchain.delete_n_account_address(auth_.name, option_.count);
        acc->set_hd_index(acc->get_hd_index() - option_.count);
        blockchain.safe_store_account(*acc, std::vector<std::shared_ptr<account_address>>{});
        jv_output["status"]= "address removed successfully";
    }
    else if ((option_.operation == "add") || (option_.operation == ""))
    {
        std::string mnemonic;
        acc->get_mnemonic(auth_.auth, mnemonic);
        if (mnemonic.empty()) {
            throw mnemonicwords_empty_exception("mnemonic empty");
        }

        //split mnemonic to vector words
        auto&& words = bc::split(mnemonic, " ", true); // with trim

        if ((words.size() % bc::wallet::mnemonic_word_multiple) != 0) {
            throw mnemonicwords_amount_exception{"invalid size of backup words."};
        }

        Json::Value addresses;
        
        std::vector<std::shared_ptr<account_address>> account_addresses;
        account_addresses.reserve(option_.count);
        const auto seed = decode_mnemonic(words);
        libbitcoin::config::base16 bs(seed);
        const data_chunk& ds = static_cast<const data_chunk&>(bs);
        const auto prefixes = bc::wallet::hd_private::to_prefixes(76066276, 0);//76066276 is HD private key version
        const bc::wallet::hd_private private_key(ds, prefixes);

        // mainnet payment address version
        auto payment_version = 68;
        if (blockchain.chain_settings().use_testnet_rules) {
            // testnucnayment address version
            payment_version = 127 ;
        }

        for (uint32_t idx = 0; idx < option_.count; idx++ ) {

            auto addr = std::make_shared<bc::chain::account_address>();
            addr->set_name(auth_.name);

            const auto child_private_key = private_key.derive_private(acc->get_hd_index());
            auto hk = child_private_key.encoded();

            // Create the private key from hd_key and the public version.
            const auto derive_private_key = bc::wallet::hd_private(hk, prefixes);
            auto pk = encode_base16(derive_private_key.secret());

            addr->set_prv_key(pk.c_str(), auth_.auth);
            // not store public key now
            ec_compressed point;
            libbitcoin::secret_to_public(point, derive_private_key.secret());

            // Serialize to the original compression state.
            auto ep =  ec_public(point, true);

            payment_address pa(ep, payment_version);

            addr->set_address(pa.encoded());
            addr->set_status(1); // 1 -- enable address

            acc->increase_hd_index();
            addr->set_hd_index(acc->get_hd_index());
            account_addresses.push_back(addr);

            addresses.append(addr->get_address());
        }

        blockchain.safe_store_account(*acc, account_addresses);

        // write to output json
        
        if(addresses.isNull())
            addresses.resize(0);  
        jv_output = addresses;
    }
    else
    {
        jv_output = string("Invalid operation [") + option_.operation + "].";
    }
    
    

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

