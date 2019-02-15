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

/************************ createtoken *************************/
struct non_negative_uint64
{
  public:
    uint64_t volume;
};

void validate(boost::any &v, const std::vector<std::string> &values,
              non_negative_uint64 *, int);

class createtoken : public command_extension
{
  public:
    static const char *symbol() { return "createtoken"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ctgy_extension & bs) == bs; }
    const char *description() override { return "createtoken "; }

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
            value<std::string>(&auth_.name)->required(),
            BX_WALLET_NAME)(
            "WALLET_AUTH",
            value<std::string>(&auth_.auth)->required(),
            BX_WALLET_AUTH)(
            "rate,r",
            value<int32_t>(&option_.registersecondarytoken_threshold),
            "The percent threshold value when you secondary issue. \
             0,  not allowed to secondary issue;  \
            -1,  the token can be secondary issue freely; \
            [1, 100], the token can be secondary issue when own percentage greater than or equal to this value. \
            Defaults to 0.")(
            "symbol,s",
            value<std::string>(&option_.symbol)->required(),
            "The token symbol, global uniqueness, only supports UPPER-CASE alphabet and dot(.), eg: -.LAPTOP, dot separates prefix '-', It's impossible to create any token named with '-' prefix, but this issuer.")(
            "issuer,i",
            value<std::string>(&option_.issuer)->required(),
            "Issue must be specified as a UID symbol.")(
            "volume,v",
            value<non_negative_uint64>(&option_.maximum_supply)->required(),
            "The token maximum supply volume, with unit of integer bits.")(
            "decimalnumber,n",
            value<uint32_t>(&option_.decimal_number),
            "The token amount decimal number, defaults to 0.")(
            "description,d",
            value<std::string>(&option_.description),
            "The token data chuck, defaults to empty string.");

        return options;
    }

    void set_defaults_from_config(po::variables_map &variables) override
    {
    }

    console_result invoke(Json::Value &jv_output,
                          libbitcoin::server::server_node &node) override;

    struct argument
    {
    } argument_;

    struct option
    {
        option() : symbol(""),
                   maximum_supply{0},
                   decimal_number(0),
                   registersecondarytoken_threshold(0),
                   issuer(""),
                   description(""){};

        std::string symbol;
        non_negative_uint64 maximum_supply;
        uint32_t decimal_number;
        int32_t registersecondarytoken_threshold;
        std::string issuer;
        std::string description;
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
