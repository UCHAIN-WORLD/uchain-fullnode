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


/************************ showvote *************************/

class showvote: public command_extension
{
public:
    static const char* symbol(){ return "showvote";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "show vote amount of uid from a START_HEIGHT to a END_HEIGHT"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("UID", 1)
            .add("START_HEIGHT", 1)
            .add("END_HEIGHT", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.uid, "UID", variables, input, raw);
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
            "UID",
            value<std::string>(&argument_.uid)->required(),
            "uid"
        )
        (
            "START_HEIGHT",
            value<uint64_t>(&argument_.startheight)->required(),
            "The start height of blockchain."
        )
        (
            "END_HEIGHT",
            value<uint64_t>(&argument_.endheight)->required(),
            "The end height of blockchain(not include)."
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
        std::string uid;
        uint64_t startheight;
        uint64_t endheight;
    } argument_;
};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

