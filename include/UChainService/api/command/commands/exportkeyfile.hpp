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

/************************ exportwalletasfile *************************/

class exportkeyfile: public command_extension
{
public:
    static const char* symbol(){ return "exportkeyfile";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "exportkeyfile"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLET_NAME", 1)
            .add("WALLET_AUTH", 1)
            .add("LASTWORD", 1)
            .add("DESTINATION", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLET_NAME", variables, input, raw);
        load_input(auth_.auth, "WALLET_AUTH", variables, input, raw);
        load_input(argument_.last_word, "LASTWORD", variables, input, raw);
        load_input(argument_.dst, "DESTINATION", variables, input, raw);
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
            "WALLET_NAME",
            value<std::string>(&auth_.name)->required(),
            BX_WALLET_NAME
        )
        (
            "WALLET_AUTH",
            value<std::string>(&auth_.auth)->required(),
            BX_WALLET_AUTH
        )
        (
            "LASTWORD",
            value<std::string>(&argument_.last_word)->required(),
            "The last word of your master private-key phrase."
        )
        (
            "DESTINATION",
            value<boost::filesystem::path>(&argument_.dst)->default_value(""),
            "The keyfile storage path to."
        )
        (
            "data,d",
            value<bool>(&option_.is_data)->default_value(false)->zero_tokens(),
            "If specified, the keyfile content will be append to the report, rather than to local file specified by DESTINATION."
        );
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
        std::string last_word;
        boost::filesystem::path dst;
    } argument_;

    struct option
    {
        bool is_data;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

