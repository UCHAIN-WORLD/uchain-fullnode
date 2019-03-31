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
#include <UChainService/api/command/commands/showbalances.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/command_assistant.hpp>
#include <UChainService/api/command/base_helper.hpp>
#include <UChainService/api/command/exception.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

using namespace bc::explorer::config;

/************************ showbalances *************************/

console_result showbalances::invoke(Json::Value &jv_output,
                                    libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    blockchain.is_wallet_passwd_valid(auth_.name, auth_.auth);

    Json::Value all_balances;

    auto vaddr = blockchain.get_wallet_addresses(auth_.name);
    if (!vaddr)
    {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    if (!option_.greater && option_.non_zero)
    {
        option_.greater = 1;
    }

    if (option_.deposited)
    {
        auto deposited_balances = std::make_shared<deposited_balance::list>();

        for (auto &i : *vaddr)
        {
            auto waddr = bc::wallet::payment_address(i.get_address());
            sync_fetch_deposited_balance(waddr, blockchain, deposited_balances);
        }

        for (auto &balance : *deposited_balances)
        {
            // non-zero lesser
            if (option_.lesser)
            {
                if (balance.balance > option_.lesser || balance.balance < option_.greater)
                {
                    continue;
                }
            }
            else
            {
                if (balance.balance < option_.greater)
                {
                    continue;
                }
            }

            Json::Value json_balance;
            json_balance["address"] = balance.address;
            json_balance["deposited_balance"] = balance.balance;
            json_balance["bonus_balance"] = balance.bonus;
            json_balance["deposited_height"] = balance.deposited_height;
            json_balance["expiration_height"] = balance.expiration_height;
            json_balance["tx_hash"] = balance.tx_hash;

            all_balances.append(json_balance);
        }
    }
    else
    {
        for (auto &i : *vaddr)
        {
            balances addr_balance{0, 0, 0, 0};
            auto waddr = bc::wallet::payment_address(i.get_address());
            sync_fetchbalance(waddr, blockchain, addr_balance);

            // non-zero lesser
            if (option_.lesser)
            {
                if (addr_balance.unspent_balance > option_.lesser || addr_balance.unspent_balance < option_.greater)
                {
                    continue;
                }
            }
            else
            {
                if (addr_balance.unspent_balance < option_.greater)
                {
                    continue;
                }
            }

            Json::Value address_balance;
            address_balance["address"] = i.get_address();

            if (get_api_version() == 1)
            {
                address_balance["confirmed"] += addr_balance.confirmed_balance;
                address_balance["received"] += addr_balance.total_received;
                address_balance["unspent"] += addr_balance.unspent_balance;
                address_balance["available"] += (addr_balance.unspent_balance - addr_balance.frozen_balance);
                address_balance["frozen"] += addr_balance.frozen_balance;
            }
            else
            {
                address_balance["confirmed"] = addr_balance.confirmed_balance;
                address_balance["received"] = addr_balance.total_received;
                address_balance["unspent"] = addr_balance.unspent_balance;
                address_balance["available"] = (addr_balance.unspent_balance - addr_balance.frozen_balance);
                address_balance["frozen"] = addr_balance.frozen_balance;
            }

            if (get_api_version() <= 2)
            {
                Json::Value target_balance;
                target_balance["balance"] = address_balance;
                all_balances.append(target_balance);
            }
            else
            {
                all_balances.append(address_balance);
            }
        }
    }

    auto &aroot = jv_output;
    if (get_api_version() == 1 && all_balances.isNull())
    { //compatible for v1
        aroot["balances"] = "";
    }
    else if (get_api_version() <= 2)
    {
        aroot["balances"] = all_balances;
    }
    else
    {
        if (all_balances.isNull())
            all_balances.resize(0);

        aroot = all_balances;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
