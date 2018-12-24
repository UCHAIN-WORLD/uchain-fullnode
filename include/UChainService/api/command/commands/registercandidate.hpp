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


/************************ registercandidate *************************/

class registercandidate : public command_extension
{
public:
    static const char* symbol(){ return "registercandidate";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "Register Candidate"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLETNAME", 1)
            .add("WALLETAUTH", 1)
            .add("TOUID", 1)
            .add("NODEADDRESS", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLETNAME", variables, input, raw);
        load_input(auth_.auth, "WALLETAUTH", variables, input, raw);
        load_input(argument_.to, "TOUID", variables, input, raw);
        load_input(argument_.symbol, "NODEADDRESS", variables, input, raw);
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
            "TOUID",
            value<std::string>(&argument_.to)->required(),
            "Target uid"
        )
        (
            "NODEADDRESS",
            value<std::string>(&argument_.symbol)->default_value(""),
            "The target node address[x.x.x.x:port]."
        )
        (
            "content,c",
            value<std::string>(&option_.content)->default_value(""),
            "Content of candidate"
        )
        /*(
            "candidates,m",
            value<std::vector<std::string>>(&option_.multicandidates),
            "List of symbol and content pair. Symbol and content are separated by a ':'"
        )*/
        (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(bc::min_fee_to_register_uid),
            "Transaction fee. defaults to 100 UCN."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        std::string to;
        std::string symbol;
        uint64_t fee;
    } argument_;

    struct option
    {
        std::string content;
        //std::vector<std::string> multicandidates;
    } option_;

private:
    void check_symbol_content(const std::string& symbol, const std::string& content);
};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

