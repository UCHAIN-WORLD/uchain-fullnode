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

#include <boost/algorithm/string.hpp>
#include <UChain/explorer/json_helper.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/importwallet.hpp>
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

console_result importwallet::invoke(Json::Value &jv_output,
                                    libbitcoin::server::server_node &node)
{

    // parameter wallet name check
    auto &blockchain = node.chain_impl();
    if (blockchain.is_wallet_exist(auth_.name))
        throw wallet_existed_exception{"wallet already exist"};

#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        option_.passwd.length() > 128 || option_.passwd.length() < 6)
        throw argument_exceed_limit_exception{"name length in [3, 128], password length in [6, 128]"};
#endif

    if (argument_.words.size() == 1)
    {
        argument_.words = bc::split(argument_.words[0], " ", true);
    }

    // are vliad mnemonic words.
    auto &&seed = get_mnemonic_to_seed(option_.language, argument_.words);
    // is vliad seed.
    auto &&hd_pri_key = get_hd_new(seed);

    auto &&mnemonic = bc::join(argument_.words);

    // create wallet
    auto acc = std::make_shared<bc::chain::wallet>();
    acc->set_name(auth_.name);
    acc->set_passwd(option_.passwd);
    acc->set_mnemonic(mnemonic, option_.passwd);
    //acc->set_hd_index(option_.hd_index); // hd_index updated in addaddress

    // flush to db
    blockchain.store_wallet(acc);

    // generate all wallet address
    auto &&str_idx = std::to_string(option_.hd_index);
    const char *cmds2[]{"addaddress", auth_.name.c_str(), option_.passwd.c_str(), "-n", str_idx.c_str()};
    Json::Value addresses;

    if (dispatch_command(5, cmds2, addresses, node, get_api_version()) != console_result::okay)
    {
        throw address_generate_exception{"addaddress got exception."};
    }

    if (get_api_version() <= 2)
    {
        if (get_api_version() == 1)
        {
            jv_output["hd_index"] += option_.hd_index;
            if (option_.hd_index == 1)
            {
                Json::Value addr;
                addr.append(addresses.asString());
                jv_output["addresses"] = addr;
            }
            else
            {
                jv_output["addresses"] = addresses["addresses"];
            }
        }
        else if (get_api_version() == 2)
        {
            jv_output["hd_index"] = option_.hd_index;
            jv_output["addresses"] = addresses["addresses"];
        }

        jv_output["name"] = auth_.name;
        jv_output["mnemonic"] = mnemonic;
    }
    else
    {
        config::json_helper::wallet_info acc(auth_.name, mnemonic, addresses);
        jv_output = config::json_helper(get_api_version()).prop_list(acc);
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
