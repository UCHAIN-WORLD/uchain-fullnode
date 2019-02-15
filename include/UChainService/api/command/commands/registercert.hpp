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

/************************ registercert *************************/

class registercert : public command_extension
{
  public:
    static const char *symbol() { return "registercert"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ex_online & bs) == bs; }
    const char *description() override { return "registercert supports define an token certification."; }

    arguments_metadata &load_arguments() override
    {
        return get_argument_metadata()
            .add("WALLET_NAME", 1)
            .add("WALLET_AUTH", 1)
            .add("TOUID", 1)
            .add("SYMBOL", 1)
            .add("CERT", 1);
    }

    void load_fallbacks(std::istream &input,
                        po::variables_map &variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "WALLET_NAME", variables, input, raw);
        load_input(auth_.auth, "WALLET_AUTH", variables, input, raw);
        load_input(argument_.to, "TOUID", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
        load_input(argument_.cert, "CERT", variables, input, raw);
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
            "TOUID",
            value<std::string>(&argument_.to)->required(),
            "The UID will own this cert.")(
            "SYMBOL",
            value<std::string>(&argument_.symbol)->required(),
            "Cert Symbol/Name.")(
            "CERT",
            value<std::string>(&argument_.cert)->required(),
            "Cert type name can be: NAMING: cert of naming right of domain. The owner of domain cert can issue this type of cert by registercert with symbol like “domain.XYZ”(domain is the symbol of domain cert).")(
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(bc::min_tx_fee),
            "Transaction fee. defaults to 200000 UCN bits.");

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
        std::string cert;
        uint64_t fee;
    } argument_;

    struct option
    {
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
