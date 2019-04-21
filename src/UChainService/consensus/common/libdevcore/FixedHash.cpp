/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file FixedHash.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <UChainService/consensus/libdevcore/FixedHash.h>
#include <ctime>
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace libbitcoin;
#ifdef __MINGW32__
boost::random_device libbitcoin::s_fixedHashEngine;
#else
std::random_device libbitcoin::s_fixedHashEngine;
#endif

h128 libbitcoin::fromUUID(std::string const &_uuid)
{
    try
    {
        return h128(boost::replace_all_copy(_uuid, "-", ""));
    }
    catch (...)
    {
        return h128();
    }
}

std::string libbitcoin::toUUID(h128 const &_uuid)
{
    std::string ret = toHex(_uuid.ref());
    for (unsigned i : {20, 16, 12, 8})
        ret.insert(ret.begin() + i, '-');
    return ret;
}
