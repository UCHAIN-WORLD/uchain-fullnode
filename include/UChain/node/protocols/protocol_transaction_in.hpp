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
#ifndef UC_NODE_PROTOCOL_TRANSACTION_IN_HPP
#define UC_NODE_PROTOCOL_TRANSACTION_IN_HPP

#include <memory>
#include <UChain/blockchain.hpp>
#include <UChain/network.hpp>
#include <UChain/node/define.hpp>

namespace libbitcoin
{
namespace node
{

class BCN_API protocol_transaction_in
    : public network::protocol_events,
      track<protocol_transaction_in>
{
  public:
    typedef std::shared_ptr<protocol_transaction_in> ptr;

    /// Construct a transaction protocol instance.
    protocol_transaction_in(network::p2p &network,
                            network::channel::ptr channel, blockchain::block_chain &blockchain,
                            blockchain::tx_pool &pool);

    ptr do_subscribe();

    /// Start the protocol.
    virtual void start();

  private:
    typedef chain::point::indexes index_list;
    typedef message::get_data::ptr get_data_ptr;
    typedef message::inventory::ptr inventory_ptr;
    typedef message::tx_message::ptr transaction_ptr;
    typedef message::block_msg::ptr_list block_ptr_list;
    typedef message::block_msg::ptr block_ptr;

    void send_get_data(const code &ec, get_data_ptr message);
    void handle_filter_floaters(const code &ec, get_data_ptr message);
    bool handle_receive_inventory(const code &ec, inventory_ptr message);
    bool handle_receive_transaction(const code &ec, transaction_ptr message);
    void handle_store_confirmed(const code &ec, transaction_ptr message);
    void handle_store_validated(const code &ec, transaction_ptr message,
                                const index_list &unconfirmed);
    bool handle_reorganized(const code &ec, size_t fork_point,
                            const block_ptr_list &incoming, const block_ptr_list &outgoing);

    void handle_stop(const code &);

    blockchain::block_chain &blockchain_;
    blockchain::tx_pool &pool_;
    const bool relay_from_peer_;
    const bool peer_suports_memory_pool_;
    const bool refresh_pool_;
};

} // namespace node
} // namespace libbitcoin

#endif
