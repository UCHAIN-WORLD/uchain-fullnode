/**
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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


#include <functional>
#include <memory>
#include <string>
#include <array>

#include <UChain/explorer/command.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/command_extension.hpp>
#include <UChainService/api/command/command_extension_func.hpp>
#include <UChainService/api/command/commands/shutdown.hpp>
#include <UChainService/api/command/commands/stopmining.hpp>
#include <UChainService/api/command/commands/startmining.hpp>
#include <UChainService/api/command/commands/showinfo.hpp>
#include <UChainService/api/command/commands/showblockheight.hpp>
#include <UChainService/api/command/commands/showpeerinfo.hpp>
#include <UChainService/api/command/commands/showaddressucn.hpp>
#include <UChainService/api/command/commands/addnode.hpp>
#include <UChainService/api/command/commands/showmininginfo.hpp>
#include <UChainService/api/command/commands/showblockheader.hpp>
#include <UChainService/api/command/commands/showheaderext.hpp>
#include <UChainService/api/command/commands/showtx.hpp>
#include <UChainService/api/command/commands/exportkeyfile.hpp>
#include <UChainService/api/command/commands/importkeyfile.hpp>
#include <UChainService/api/command/commands/importaccount.hpp>
#include <UChainService/api/command/commands/createaccount.hpp>
#include <UChainService/api/command/commands/checkaccountinfo.hpp>
#include <UChainService/api/command/commands/deleteaccount.hpp>
#include <UChainService/api/command/commands/showaddresses.hpp>
#include <UChainService/api/command/commands/showminers.hpp>
#include <UChainService/api/command/commands/addaddress.hpp>
#include <UChainService/api/command/commands/showblock.hpp>
#include <UChainService/api/command/commands/validateaddress.hpp>
#include <UChainService/api/command/commands/showbalances.hpp>
#include <UChainService/api/command/commands/showbalance.hpp>
#include <UChainService/api/command/commands/showtxs.hpp>
#include <UChainService/api/command/commands/deposit.hpp>
#include <UChainService/api/command/commands/showtokens.hpp>
#include <UChainService/api/command/commands/showtoken.hpp>
#include <UChainService/api/command/commands/registersecondarytoken.hpp>
#include <UChainService/api/command/commands/showaddresstoken.hpp>
#include <UChainService/api/command/commands/showaccounttoken.hpp>
#include <UChainService/api/command/commands/showtokenview.hpp>
#include <UChainService/api/command/commands/createtoken.hpp>
#include <UChainService/api/command/commands/registercard.hpp>
#include <UChainService/api/command/commands/transfercard.hpp>
#include <UChainService/api/command/commands/showcards.hpp>
#include <UChainService/api/command/commands/showcard.hpp>
#include <UChainService/api/command/commands/registeruid.hpp>
#include <UChainService/api/command/commands/sendto.hpp>
#include <UChainService/api/command/commands/sendfrom.hpp>
#include <UChainService/api/command/commands/sendtomulti.hpp>
#include <UChainService/api/command/commands/sendtokento.hpp>
#include <UChainService/api/command/commands/sendtokenfrom.hpp>
#include <UChainService/api/command/commands/swaptoken.hpp>
#include <UChainService/api/command/commands/vote.hpp>
#include <UChainService/api/command/commands/showuids.hpp>
#include <UChainService/api/command/commands/deletetoken.hpp>
#include <UChainService/api/command/commands/registertoken.hpp>
#include <UChainService/api/command/commands/destroy.hpp>
#include <UChainService/api/command/commands/transfercert.hpp>
#include <UChainService/api/command/commands/registercert.hpp>
#include <UChainService/api/command/commands/showwork.hpp>
#include <UChainService/api/command/commands/submitwork.hpp>
#include <UChainService/api/command/commands/setminingaccount.hpp>
#include <UChainService/api/command/commands/changepass.hpp>
#include <UChainService/api/command/commands/showmemorypool.hpp>
#include <UChainService/api/command/commands/createmultisigtx.hpp>
#include <UChainService/api/command/commands/createrawtx.hpp>
#include <UChainService/api/command/commands/decoderawtx.hpp>
#include <UChainService/api/command/commands/deletemultisigaddress.hpp>
#include <UChainService/api/command/commands/createmultisigaddress.hpp>
#include <UChainService/api/command/commands/checkpublickey.hpp>
#include <UChainService/api/command/commands/showmultisigaddress.hpp>
#include <UChainService/api/command/commands/sendrawtx.hpp>
#include <UChainService/api/command/commands/signmultisigtx.hpp>
#include <UChainService/api/command/commands/signrawtx.hpp>
#include <UChainService/api/command/commands/transferuid.hpp>
#include <UChainService/api/command/commands/showuid.hpp>


namespace libbitcoin {
namespace explorer {


void broadcast_extension(const function<void(shared_ptr<command>)> func, std::ostream& os)
{
    using namespace std;
    using namespace commands;

    os <<"\r\n";
    // account
    func(make_shared<createaccount>());
    func(make_shared<checkaccountinfo>());
    func(make_shared<deleteaccount>());
    func(make_shared<importaccount>());
    func(make_shared<changepass>());
    func(make_shared<addaddress>());
    func(make_shared<validateaddress>());
    func(make_shared<showaddresses>());
    func(make_shared<exportkeyfile>());
    func(make_shared<importkeyfile>());

    os <<"\r\n";
    // system
    func(make_shared<shutdown>());
    func(make_shared<showinfo>());
    func(make_shared<addnode>());
    func(make_shared<showpeerinfo>());

    // miming
    func(make_shared<startmining>());
    func(make_shared<stopmining>());
    func(make_shared<showmininginfo>());
    func(make_shared<setminingaccount>());
    func(make_shared<showminers>());
    func(make_shared<showwork>());
    func(make_shared<submitwork>());
    func(make_shared<showmemorypool>());

    os <<"\r\n";
    // block & tx
    func(make_shared<showblockheight>());
    func(make_shared<showblock>());
    func(make_shared<showblockheader>());
    func(make_shared<showheaderext>());
    func(make_shared<showtx>());
    func(make_shared<showtxs>());
    os <<"\r\n";

    // raw tx and multi-sig
    func(make_shared<createrawtx>());
    func(make_shared<decoderawtx>());
    func(make_shared<signrawtx>());
    func(make_shared<sendrawtx>());
    func(make_shared<checkpublickey>());
    func(make_shared<createmultisigtx>());
    func(make_shared<createmultisigaddress>());
    func(make_shared<showmultisigaddress>());
    func(make_shared<deletemultisigaddress>());
    func(make_shared<signmultisigtx>());

    os <<"\r\n";
    // ucn
    func(make_shared<sendto>());
    func(make_shared<sendtomulti>());
    func(make_shared<sendfrom>());
    func(make_shared<deposit>());
    func(make_shared<showbalances>());
    func(make_shared<showbalance>());
    func(make_shared<showaddressucn>());

    //os <<"\r\n";
    // token
    func(make_shared<createtoken>());
    func(make_shared<deletetoken>());
    func(make_shared<registertoken>());
    /*func(make_shared<registersecondarytoken>());*/
    func(make_shared<sendtokento>());
    func(make_shared<sendtokenfrom>());
    func(make_shared<showtokens>());
    func(make_shared<showtoken>());
    func(make_shared<showaccounttoken>());
    // func(make_shared<showtokenview>());
    func(make_shared<showaddresstoken>());
    func(make_shared<destroy>());
    func(make_shared<swaptoken>());
    func(make_shared<vote>());

    //os <<"\r\n";
    // cert
    /*func(make_shared<registercert>());
    func(make_shared<transfercert>());*/

    //os <<"\r\n";
    // mit
    /*func(make_shared<registercard>());
    func(make_shared<transfercard>());
    func(make_shared<showcards>());
    func(make_shared<showcard>());

    os <<"\r\n";*/
    //uid
    /*func(make_shared<registeruid>());
    func(make_shared<transferuid>());
    func(make_shared<showuids>());
    func(make_shared<showuid>());*/
}

shared_ptr<command> find_extension(const string& symbol)
{
    using namespace std;
    using namespace commands;

    // account
    if (symbol == createaccount::symbol())
        return make_shared<createaccount>();
    if (symbol == checkaccountinfo::symbol())
        return make_shared<checkaccountinfo>();
    if (symbol == deleteaccount::symbol())
        return make_shared<deleteaccount>();
    if (symbol == changepass::symbol())
        return make_shared<changepass>();
    if (symbol == validateaddress::symbol())
        return make_shared<validateaddress>();
    if (symbol == addaddress::symbol())
        return make_shared<addaddress>();
    if (symbol == showaddresses::symbol())
        return make_shared<showaddresses>();
    if (symbol == importaccount::symbol())
        return make_shared<importaccount>();
    if (symbol == exportkeyfile::symbol() || symbol == "exportaccountasfile")
        return make_shared<exportkeyfile>();
    if (symbol == importkeyfile::symbol() || symbol == "importaccountfromfile")
        return make_shared<importkeyfile>();

    // system
    if (symbol == shutdown::symbol())
        return make_shared<shutdown>();
    if (symbol == showinfo::symbol())
        return make_shared<showinfo>();
    if (symbol == addnode::symbol())
        return make_shared<addnode>();
    if (symbol == showpeerinfo::symbol())
        return make_shared<showpeerinfo>();

    // mining
    if (symbol == stopmining::symbol() || symbol == "stop")
        return make_shared<stopmining>();
    if (symbol == startmining::symbol() || symbol == "start")
        return make_shared<startmining>();
    if (symbol == setminingaccount::symbol())
        return make_shared<setminingaccount>();
    if (symbol == showmininginfo::symbol())
        return make_shared<showmininginfo>();
    if ((symbol == showwork::symbol()) || (symbol == "eth_showwork"))
        return make_shared<showwork>();
    if ((symbol == submitwork::symbol()) || ( symbol == "eth_submitWork"))
        return make_shared<submitwork>();
    if (symbol == showmemorypool::symbol())
        return make_shared<showmemorypool>();
    if (symbol == showminers::symbol())
        return make_shared<showminers>();

    // block & tx
    if (symbol == showblockheight::symbol())
        return make_shared<showblockheight>();
    if (symbol == "fetch-height")
        return make_shared<showblockheight>(symbol);
    if (symbol == showblock::symbol())
        return make_shared<showblock>();
    if (symbol == "getbestblockhash")
        return make_shared<showblockheader>(symbol);
    if (symbol == showblockheader::symbol() || symbol == "fetch-header" || symbol == "getbestblockheader")
        return make_shared<showblockheader>();
    if (symbol == showheaderext::symbol())
        return make_shared<showheaderext>();
    if (symbol == showtx::symbol() || symbol == "gettransaction")
        return make_shared<showtx>();
    if (symbol == "fetch-tx")
        return make_shared<showtx>(symbol);
    if (symbol == showtxs::symbol())
        return make_shared<showtxs>();

    // raw tx
    if (symbol == createrawtx::symbol())
        return make_shared<createrawtx>();
    if (symbol == decoderawtx::symbol())
        return make_shared<decoderawtx>();
    if (symbol == signrawtx::symbol())
        return make_shared<signrawtx>();
    if (symbol == sendrawtx::symbol())
        return make_shared<sendrawtx>();

    // multi-sig
    if (symbol == checkpublickey::symbol())
        return make_shared<checkpublickey>();
    if (symbol == createmultisigaddress::symbol())
        return make_shared<createmultisigaddress>();
    if (symbol == showmultisigaddress::symbol())
        return make_shared<showmultisigaddress>();
    if (symbol == deletemultisigaddress::symbol())
        return make_shared<deletemultisigaddress>();
    if (symbol == createmultisigtx::symbol())
        return make_shared<createmultisigtx>();
    if (symbol == signmultisigtx::symbol())
        return make_shared<signmultisigtx>();

    // ucn
    if (symbol == showbalances::symbol())
        return make_shared<showbalances>();
    if (symbol == showbalance::symbol())
        return make_shared<showbalance>();
    if (symbol == showaddressucn::symbol() || symbol == "fetch-balance")
        return make_shared<showaddressucn>();
    if (symbol == deposit::symbol())
        return make_shared<deposit>();
    if (symbol == sendto::symbol() || symbol == "uidsendto")
        return make_shared<sendto>();
    if (symbol == sendtomulti::symbol() || symbol == "uidsendtomulti")
        return make_shared<sendtomulti>();
    if (symbol == sendfrom::symbol() || symbol == "uidsendfrom")
        return make_shared<sendfrom>();

    // token
    if (symbol == createtoken::symbol())
        return make_shared<createtoken>();
    if (symbol == deletetoken::symbol() || symbol == "deletetoken" )
        return make_shared<deletetoken>();
    if (symbol == showtokens::symbol())
        return make_shared<showtokens>();
    if (symbol == showtoken::symbol())
        return make_shared<showtoken>();
    if (symbol == showaccounttoken::symbol())
        return make_shared<showaccounttoken>();
    // if (symbol == showtokenview::symbol())
    //     return make_shared<showtokenview>();
    if (symbol == showaddresstoken::symbol())
        return make_shared<showaddresstoken>();
    if (symbol == registertoken::symbol())
        return make_shared<registertoken>();
    /*if (symbol == registersecondarytoken::symbol() || (symbol == "additionalissue") )
        return make_shared<registersecondarytoken>();*/
    if (symbol == sendtokento::symbol() || symbol == "uidsendtokento")
        return make_shared<sendtokento>();
    if (symbol == sendtokenfrom::symbol() || symbol == "uidsendtokenfrom")
        return make_shared<sendtokenfrom>();
    if (symbol == destroy::symbol())
        return make_shared<destroy>();
    if (symbol == swaptoken::symbol())
        return make_shared<swaptoken>();
    if (symbol == vote::symbol())
        return make_shared<vote>();

    // cert
    /*if (symbol == transfercert::symbol())
        return make_shared<transfercert>();
    if (symbol == registercert::symbol())
        return make_shared<registercert>();*/

    // mit
    /*if (symbol == registercard::symbol())
        return make_shared<registercard>();
    if (symbol == transfercard::symbol())
        return make_shared<transfercard>();
    if (symbol == showcards::symbol())
        return make_shared<showcards>();
    if (symbol == showcard::symbol())
        return make_shared<showcard>();*/

    // uid
    /*if (symbol == registeruid::symbol())
        return make_shared<registeruid>();
    if (symbol == transferuid::symbol())
        return make_shared<transferuid>();
    if (symbol == showuids::symbol())
        return make_shared<showuids>();
    if (symbol == showuid::symbol())
        return make_shared<showuid>();*/

    return nullptr;
}

std::string formerly_extension(const string& former)
{
    return "";
}

} // namespace explorer
} // namespace libbitcoin

