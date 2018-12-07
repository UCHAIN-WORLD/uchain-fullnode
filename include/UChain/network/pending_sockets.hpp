/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain.
 *
 * UChain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_NETWORK_PENDING_SOCKETS_HPP
#define UC_NETWORK_PENDING_SOCKETS_HPP

#include <vector>
#include <UChain/bitcoin.hpp>
#include <UChain/network/define.hpp>
#include <UChain/network/socket.hpp>

namespace libbitcoin {
namespace network {

/// Class to manage a pending socket pool, thread and lock safe.
class BCT_API pending_sockets
{
public:
    pending_sockets();
    ~pending_sockets();

    /// This class is not copyable.
    pending_sockets(const pending_sockets&) = delete;
    void operator=(const pending_sockets&) = delete;

    virtual void clear();
    virtual void store(socket::ptr socket);
    virtual void remove(socket::ptr socket);

private:
    typedef std::vector<socket::ptr> list;

    bool safe_clear();
    bool safe_store(socket::ptr socket);
    bool safe_remove(socket::ptr socket);

    list sockets_;
    mutable shared_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif

