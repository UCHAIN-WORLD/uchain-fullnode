/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
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
#ifndef BX_HELP_HPP
#define BX_HELP_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <UChain/coin.hpp>
#include <UChain/explorer/command.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/generated.hpp>
#include <UChain/explorer/config/address.hpp>
#include <UChain/explorer/config/algorithm.hpp>
#include <UChain/explorer/config/btc.hpp>
#include <UChain/explorer/config/byte.hpp>
#include <UChain/explorer/config/cert_key.hpp>
#include <UChain/explorer/config/ec_private.hpp>
#include <UChain/explorer/config/encoding.hpp>
#include <UChain/explorer/config/endorsement.hpp>
#include <UChain/explorer/config/hashtype.hpp>
#include <UChain/explorer/config/hd_key.hpp>
#include <UChain/explorer/config/header.hpp>
#include <UChain/explorer/config/input.hpp>
#include <UChain/explorer/config/language.hpp>
#include <UChain/explorer/config/output.hpp>
#include <UChain/explorer/config/raw.hpp>
#include <UChain/explorer/config/script.hpp>
#include <UChain/explorer/config/signature.hpp>
#include <UChain/explorer/config/transaction.hpp>
#include <UChain/explorer/config/wrapper.hpp>
#include <UChain/explorer/utility.hpp>

/********* GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY **********/

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/**
 * Class to implement the help command.
 */
class BCX_API help
    : public command
{
  public:
    /**
     * The symbolic (not localizable) command name, lower case.
     */
    static const char *symbol()
    {
        return "help";
    }

    /**
     * The member symbolic (not localizable) command name, lower case.
     */
    virtual const char *name()
    {
        return help::symbol();
    }

    /**
     * The localizable command category name, upper case.
     */
    virtual const char *category()
    {
        return "META";
    }

    /**
     * The localizable command description.
     */
    virtual const char *description()
    {
        return "Get the list of commands.";
    }

    /**
     * Load program argument definitions.
     * A value of -1 indicates that the number of instances is unlimited.
     * @return  The loaded program argument definitions.
     */
    virtual arguments_metadata &load_arguments()
    {
        return get_argument_metadata()
            .add("COMMAND", 1);
    }

    /**
     * Load parameter fallbacks from file or input as appropriate.
     * @param[in]  input  The input stream for loading the parameters.
     * @param[in]         The loaded variables.
     */
    virtual void load_fallbacks(std::istream &input,
                                po::variables_map &variables)
    {
    }

    /**
     * Load program option definitions.
     * BUGBUG: see boost bug/fix: svn.boost.org/trac/boost/ticket/8009
     * @return  The loaded program option definitions.
     */
    virtual options_metadata &load_options()
    {
        using namespace po;
        options_description &options = get_option_metadata();
        options.add_options()(
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command.")(
            "COMMAND",
            value<std::string>(&argument_.command),
            "The command for which help is requested.");

        return options;
    }

    /**
     * Set variable defaults from configuration variable values.
     * @param[in]  variables  The loaded variables.
     */
    virtual void set_defaults_from_config(po::variables_map &variables)
    {
    }

    /**
     * Invoke the command.
     * @param[out]  output  The input stream for the command execution.
     * @param[out]  error   The input stream for the command execution.
     * @return              The appropriate console return code { -1, 0, 1 }.
     */
    virtual console_result invoke(std::ostream &output,
                                  std::ostream &cerr);

    /* Properties */

    /**
     * Get the value of the COMMAND argument.
     */
    virtual std::string &get_command_argument()
    {
        return argument_.command;
    }

    /**
     * Set the value of the COMMAND argument.
     */
    virtual void set_command_argument(
        const std::string &value)
    {
        argument_.command = value;
    }

  private:
    /**
     * Command line argument bound variables.
     * Uses cross-compiler safe constructor-based zeroize.
     * Zeroize for unit test consistency with program_options initialization.
     */
    struct argument
    {
        argument()
            : command()
        {
        }

        std::string command;
    } argument_;

    /**
     * Command line option bound variables.
     * Uses cross-compiler safe constructor-based zeroize.
     * Zeroize for unit test consistency with program_options initialization.
     */
    struct option
    {
        option()
        {
        }

    } option_;
};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

#endif
