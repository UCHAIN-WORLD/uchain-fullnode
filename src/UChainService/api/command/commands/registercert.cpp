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
#include <UChainService/api/command/commands/registercert.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChainService/api/command/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

template <typename ElemT>
struct HexTo {
    ElemT value;
    operator ElemT() const {return value;}
    friend std::istream& operator>>(std::istream& in, HexTo& out) {
        in >> std::hex >> out.value;
        return in;
    }
};

console_result registercert::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);
    boost::to_lower(argument_.cert);

    // check token symbol
    check_token_symbol(argument_.symbol);

    auto to_uid = argument_.to;
    auto to_address = get_address_from_uid(to_uid, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw address_invalid_exception{"invalid uid parameter! " + to_uid};
    }
    if (!blockchain.get_wallet_address(auth_.name, to_address)) {
        throw address_dismatch_wallet_exception{"target uid does not match wallet. " + to_uid};
    }

    // check token cert types
    auto certs_create = token_cert_ns::none;
    std::map <std::string, token_cert_type> cert_map = {
        {"naming",      token_cert_ns::naming},
        {"marriage",    token_cert_ns::marriage},
        {"kyc",         token_cert_ns::kyc}
    };
    auto iter = cert_map.find(argument_.cert);
    if (iter != cert_map.end()) {
        certs_create = iter->second;
    }
    else {
        try {
            if (argument_.cert.compare(0, 2, "0x") == 0) {
                certs_create = boost::lexical_cast<HexTo<token_cert_type>>(argument_.cert.c_str());
            }
            else {
                certs_create = boost::lexical_cast<token_cert_type>(argument_.cert.c_str());
            }

            if (certs_create < token_cert_ns::custom) {
                throw token_cert_exception("invalid token cert type " + argument_.cert);
            }
        }
        catch(boost::bad_lexical_cast const&) {
            throw token_cert_exception("invalid token cert type " + argument_.cert);
        }
    }

    if (certs_create == token_cert_ns::naming) {
        // check symbol is valid.
        auto pos = argument_.symbol.find(".");
        if (pos == std::string::npos) {
            throw token_symbol_name_exception("invalid naming cert symbol " + argument_.symbol
                + ", it should contain a dot '.'");
        }

        auto&& domain = token_cert::get_domain(argument_.symbol);
        if (!token_cert::is_valid_domain(domain)) {
            throw token_symbol_name_exception("invalid naming cert symbol " + argument_.symbol
                + ", it should contain a valid domain!");
        }

        // check domain naming cert not exist.
        if (blockchain.is_token_cert_exist(argument_.symbol, token_cert_ns::naming)) {
            throw token_cert_existed_exception(
                "naming cert '" + argument_.symbol + "' already exists on the blockchain!");
        }

        // check token not exist.
        if (blockchain.is_token_exist(argument_.symbol, false)) {
            throw token_symbol_existed_exception(
                "token symbol '" + argument_.symbol + "' already exists on the blockchain!");
        }

        // check domain cert belong to this wallet.
        bool exist = blockchain.is_token_cert_exist(domain, token_cert_ns::domain);
        if (!exist) {
            throw token_cert_notfound_exception("no domain cert '" + domain + "' found!");
        }

        auto cert = blockchain.get_wallet_token_cert(auth_.name, domain, token_cert_ns::domain);
        if (!cert) {
            throw token_cert_notowned_exception("no domain cert '" + domain + "' owned by " + auth_.name);
        }
    }

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, 0,
            certs_create, utxo_attach_type::token_cert_issue,
            asset("", to_uid)}
    };

    if (certs_create == token_cert_ns::naming) {
        auto&& domain = token_cert::get_domain(argument_.symbol);
        receiver.push_back(
            {to_address, domain, 0, 0,
                token_cert_ns::domain, utxo_attach_type::token_cert,
                asset("", to_uid)}
        );
    }

    auto helper = issuing_token_cert(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        std::move(to_address), std::move(argument_.symbol),
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

