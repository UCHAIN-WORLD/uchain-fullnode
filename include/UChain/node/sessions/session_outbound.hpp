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
#ifndef UC_NODE_SESSION_OUTBOUND_HPP
#define UC_NODE_SESSION_OUTBOUND_HPP

#include <memory>
#include <UChain/blockchain.hpp>
#include <UChain/network.hpp>
#include <UChain/node/define.hpp>

namespace libbitcoin
{
namespace node
{

/// Outbound connections session, thread safe.
class BCN_API session_outbound
    : public network::session_outbound
{
  public:
    typedef std::shared_ptr<session_outbound> ptr;

    /// Construct an instance.
    session_outbound(network::p2p &network,
                     blockchain::block_chain &blockchain,
                     blockchain::transaction_pool &pool);

    virtual void attach_handshake_protocols(network::channel::ptr channel,
                                            result_handler handle_started) override;

  protected:
    /// Overridden to attach blockchain protocols.
    void attach_protocols(network::channel::ptr channel) override;

    blockchain::block_chain &blockchain_;
    blockchain::transaction_pool &pool_;
    /*mine::miner& miner_*/
};

} // namespace node
} // namespace libbitcoin

#endif
