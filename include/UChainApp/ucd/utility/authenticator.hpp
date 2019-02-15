/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-server.
 *
 * UChain-server is free software: you can redistribute it and/or
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
#ifndef UC_SERVER_SECURE_AUTHENTICATOR_HPP
#define UC_SERVER_SECURE_AUTHENTICATOR_HPP

#include <memory>
#include <UChain/protocol.hpp>
#include <UChainApp/ucd/define.hpp>
#include <UChainApp/ucd/settings.hpp>

namespace libbitcoin
{
namespace server
{

class server_node;

class BCS_API authenticator
    : public bc::protocol::zmq::authenticator
{
public:
  typedef std::shared_ptr<authenticator> ptr;

  /// Construct an instance of the authenticator.
  authenticator(server_node &node);

  /// This class is not copyable.
  authenticator(const authenticator &) = delete;
  void operator=(const authenticator &) = delete;

  /// Apply authentication to the socket.
  bool apply(bc::protocol::zmq::socket &socket, const std::string &domain,
             bool secure);
};

} // namespace server
} // namespace libbitcoin

#endif
