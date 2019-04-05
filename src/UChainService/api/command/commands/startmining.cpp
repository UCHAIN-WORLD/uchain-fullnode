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

#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/commands/startmining.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

/************************ startmining *************************/

console_result startmining::invoke(Json::Value &jv_output,
                                   libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    auto &miner = node.miner();

    uint64_t height;
    //uint64_t rate;
    //std::string difficulty;
    uint32_t miners;
    bool is_solo_mining;
    node.miner().get_state(height, miners, /*rate, difficulty,*/ is_solo_mining);
    if (is_solo_mining)
    {
        throw setting_required_exception{"Currently mining, please use command <stopmining> to stop the running mining."};
    }

    auto str_addr = argument_.address;

    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    if (!blockchain.is_valid_address(str_addr))
    {
        throw address_invalid_exception{"invalid address parameter! " + str_addr};
    }

    /*const vector<string>& miner_address = miner.get_miner_addresses();
    if(std::find(miner_address.begin(), miner_address.end(), str_addr) == miner_address.end()) 
        throw address_invalid_exception{str_addr + " is not a miner address "};*/

    if (!blockchain.get_wallet_address(auth_.name, str_addr))
    {
        throw address_dismatch_wallet_exception{"target address does not match wallet. " + str_addr};
    }

    bc::wallet::payment_address addr(str_addr);

    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh)
    { // for multisig address
        throw argument_legality_exception{"script address parameter not allowed!"};
    }

    // start
    const auto &spaddr = blockchain.get_wallet_address(auth_.name, str_addr);
    miner.set_miner_pri_key(spaddr->get_prv_key(auth_.auth));
    if (miner.start(addr, option_.number))
    {
        if (option_.number == 0)
        {
            jv_output = "solo mining started at " + str_addr;
        }
        else
        {
            jv_output = "solo mining started at " + str_addr + ", try to mine " + std::to_string(option_.number) + " block(s).";
        }
    }
    else
    {
        throw unknown_error_exception{"solo mining startup got error"};
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
