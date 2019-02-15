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

/************************ sendtokento *************************/

class sendtokento : public command_extension
{
  public:
    static const char *symbol() { return "sendtokento"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ex_online & bs) == bs; }
    const char *description() override { return "sendtokento"; }

    arguments_metadata &load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLET_NAME", 1)
            .add("WALLET_AUTH", 1)
            .add("TO_", 1)
            .add("TOKEN", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks(std::istream &input,
                        po::variables_map &variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLET_NAME", variables, input, raw);
        load_input(auth_.auth, "WALLET_AUTH", variables, input, raw);
        load_input(argument_.to, "TO_", variables, input, raw);
        load_input(argument_.symbol, "TOKEN", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
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
            "TO_",
            value<std::string>(&argument_.to)->required(),
            "Token receiver uid/address.")(
            "TOKEN",
            value<std::string>(&argument_.symbol)->required(),
            "Token token symbol.")(
            "AMOUNT",
            value<uint64_t>(&argument_.amount)->required(),
            "Token integer bits. see token <decimal_number>.")(
            "change,c",
            value<std::string>(&option_.change)->default_value(""),
            "Change to this uid/address")(
            "model,m",
            value<std::string>(&option_.attenuation_model_param)->default_value(""),
            BX_TOKEN_OFFERING_CURVE)(
            "fee,f",
            value<uint64_t>(&option_.fee)->default_value(bc::min_tx_fee),
            "Transaction fee. defaults to 200000 UCN bits.")(
            "memo,i",
            value<std::string>(&option_.memo)->default_value(""),
            "Attached memo for this transaction.");
        return options;
    }

    void set_defaults_from_config(po::variables_map &variables) override
    {
    }

    console_result invoke(Json::Value &jv_output,
                          libbitcoin::server::server_node &node) override;

    struct argument
    {
        std::string to;
        std::string symbol;
        uint64_t amount;
    } argument_;

    struct option
    {
        std::string attenuation_model_param;
        std::string memo;
        std::string change;
        uint64_t fee;
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
