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
#include <UChain/explorer/config/hashtype.hpp>
#include <UChain/explorer/config/header.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/************************ showblockheaders *************************/

class showblockheaders : public command_extension
{
  public:
    static const char *symbol() { return "showblockheaders"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ex_online & bs) == bs; }
    const char *description() override { return "List block headers of this wallet."; }

    arguments_metadata &load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLET_NAME", 1)
            .add("WALLET_AUTH", 1);
    }

    void load_fallbacks(std::istream &input,
                        po::variables_map &variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLET_NAME", variables, input, raw);
        load_input(auth_.auth, "WALLET_AUTH", variables, input, raw);
    }

    options_metadata &load_options() override
    {
        using namespace po;
        options_description &options = get_option_metadata();
        options.add_options()(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command.")(
            "WALLET_NAME",
            value<std::string>(&auth_.name),
            BX_WALLET_NAME)(
            "WALLET_AUTH",
            value<std::string>(&auth_.auth),
            BX_WALLET_AUTH)(
            "height,e",
            value<libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>>(&option_.height)->required(),
            "Get block headers according height eg: -e start-height:end-height will return tx between [start-height, end-height], \
            support 100 headers at most.");

        return options;
    }

    void set_defaults_from_config(po::variables_map &variables) override
    {
    }

    console_result invoke(Json::Value &jv_output,
                          libbitcoin::server::server_node &node) override;

    struct argument
    {
        argument(){};
        uint64_t limit;
        uint64_t index;
    } argument_;

    struct option
    {
        option() : height(0, 0){};
        libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t> height;
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
