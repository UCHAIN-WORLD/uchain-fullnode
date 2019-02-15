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

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/************************ showcandidate *************************/

class showcandidate : public command_extension
{
  public:
    static const char *symbol() { return "showcandidate"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ex_online & bs) == bs; }
    const char *description() override { return "Get information of candidate."; }

    arguments_metadata &load_arguments() override
    {
        return get_argument_metadata()
            .add("SYMBOL", 1);
    }

    void load_fallbacks(std::istream &input,
                        po::variables_map &variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
    }

    options_metadata &load_options() override
    {
        using namespace po;
        options_description &options = get_option_metadata();
        options.add_options()(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command.")(
            "SYMBOL",
            value<std::string>(&argument_.symbol),
            "Candidate symbol. If not specified then show whole network candidate symbols.")(
            "limit,l",
            value<uint32_t>(&option_.limit)->default_value(100),
            "candidate count per page.")(
            "index,i",
            value<uint32_t>(&option_.index)->default_value(1),
            "Page index.")(
            "current,c",
            value<bool>(&option_.show_current)->default_value(false)->zero_tokens(),
            "If specified then show the lastest information of specified candidate. Default is not specified.");

        return options;
    }

    void set_defaults_from_config(po::variables_map &variables) override
    {
    }

    console_result invoke(Json::Value &jv_output,
                          libbitcoin::server::server_node &node) override;

    struct argument
    {
        argument() : symbol()
        {
        }

        std::string symbol;
    } argument_;

    struct option
    {
        bool show_current;
        uint32_t index;
        uint32_t limit;
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
