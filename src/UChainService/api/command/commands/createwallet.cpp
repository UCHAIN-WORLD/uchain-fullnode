/**
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

#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/createwallet.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/exception.hpp>
#include <UChain/explorer/commands/offline_commands_impl.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{
using namespace bc::explorer::config;

/************************ createwallet *************************/

console_result createwallet::invoke(Json::Value &jv_output,
                                    libbitcoin::server::server_node &node)
{
#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        auth_.auth.length() > 128 || auth_.auth.length() < 6)
        throw argument_legality_exception{"name length in [3, 128], password length in [6, 128]"};
#endif

    auto &blockchain = node.chain_impl();
    if (blockchain.is_wallet_exist(auth_.name))
    {
        throw wallet_existed_exception{"wallet already exist"};
    }

    auto acc = std::make_shared<bc::chain::wallet>();
    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    bc::explorer::config::language opt_language(option_.language);
    auto &&seed = get_seed();
    auto &&words_list = get_mnemonic_new(opt_language, seed);
    auto &&words = bc::join(words_list);

    acc->set_mnemonic(words, auth_.auth);

    // flush to db
    auto ret = blockchain.store_wallet(acc);

    // get 1 new sub-address by default
    std::stringstream sout("");
    Json::Value jv_temp;
    const char *cmds2[]{"addaddress", auth_.name.c_str(), auth_.auth.c_str()};

    if (dispatch_command(3, cmds2, jv_temp, node, get_api_version()) != console_result::okay)
    {
        throw address_generate_exception(sout.str());
    }

    {
        config::json_helper::wallet_info acc(auth_.name, words, jv_temp);
        jv_output = config::json_helper(get_api_version()).prop_list(acc);
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
