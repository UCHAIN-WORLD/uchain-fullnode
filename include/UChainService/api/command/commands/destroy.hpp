/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-api.
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


#pragma once
#include <UChain/explorer/define.hpp>
#include <UChainService/api/command/command_extension.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ destroy *************************/

class destroy: public command_extension
{
public:
    static const char* symbol(){ return "destroy";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "destroy token to blackhole address 1111111111111111111114oLvT2."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLETNAME", 1)
            .add("WALLETAUTH", 1)
            .add("SYMBOL", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLETNAME", variables, input, raw);
        load_input(auth_.auth, "WALLETAUTH", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
        (
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            "WALLETNAME",
            value<std::string>(&auth_.name)->required(),
            BX_WALLET_NAME
        )
        (
            "WALLETAUTH",
            value<std::string>(&auth_.auth)->required(),
            BX_WALLET_AUTH
        )
        (
            "SYMBOL",
            value<std::string>(&argument_.symbol)->required(),
            "The token will be destroyed."
        )
        (
            "AMOUNT",
            value<uint64_t>(&argument_.amount)->default_value(0),
            "Token integer bits. see token <decimal_number>."
        )
        (
            "cert,c",
            value<std::string>(&option_.cert_type)->default_value(""),
            "If specified, then only destroy related cert. Default is not specified."
        )
        (
            "candidate,m",
            value<bool>(&option_.is_candidate)->default_value(false)->zero_tokens(),
            "If specified, then only destroy related candidate. Default is not specified."
        )
        ;

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        argument(): amount(0), symbol("")
        {}

        uint64_t amount;
        std::string symbol;
    } argument_;

    struct option
    {
        option(): is_candidate(false), cert_type("")
        {}

        bool is_candidate;
        std::string cert_type;
    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

