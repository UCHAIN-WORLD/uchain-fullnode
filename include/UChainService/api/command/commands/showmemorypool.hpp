/*
 * showmemorypool.hpp
 *
 *  Created on: Jul 3, 2017
 *      Author: jiang
 */

#ifndef INCLUDE_UChain_EXPLORER_EXTENSIONS_WALLET_showmemorypool_HPP_
#define INCLUDE_UChain_EXPLORER_EXTENSIONS_WALLET_showmemorypool_HPP_

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

/************************ showmemorypool *************************/

class showmemorypool : public command_extension
{
  public:
    static const char *symbol() { return "showmemorypool"; }
    const char *name() override { return symbol(); }
    bool category(int bs) override { return (ctgy_extension & bs) == bs; }
    const char *description() override { return "Returns all transactions in memory pool."; }

    arguments_metadata &load_arguments() override
    {
        return get_argument_metadata()
            .add("ADMINNAME", 1)
            .add("ADMINAUTH", 1);
    }

    void load_fallbacks(std::istream &input,
                        po::variables_map &variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ADMINNAME", variables, input, raw);
        load_input(auth_.auth, "ADMINAUTH", variables, input, raw);
    }

    options_metadata &load_options() override
    {
        using namespace po;
        options_description &options = get_option_metadata();
        options.add_options()(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command.")(
            "json,j",
            value<bool>(&option_.json)->default_value(true),
            "Json format or Raw format, default is Json(true).")(
            "ADMINNAME",
            value<std::string>(&auth_.name),
            BX_ADMIN_NAME)(
            "ADMINAUTH",
            value<std::string>(&auth_.auth),
            BX_ADMIN_AUTH);

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
        bool json;
    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

#endif /* INCLUDE_UChain_EXPLORER_EXTENSIONS_WALLET_showmemorypool_HPP_ */
