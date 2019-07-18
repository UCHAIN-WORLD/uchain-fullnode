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
#include <UChain/node/protocols/protocol_tx_out.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <UChain/network.hpp>

namespace libbitcoin
{
namespace node
{

#define NAME "transaction"
#define CLASS protocol_tx_out

using namespace bc::blockchain;
using namespace bc::message;
using namespace bc::network;
using namespace std::placeholders;

protocol_tx_out::protocol_tx_out(p2p &network,
                                                   channel::ptr channel, block_chain &blockchain, tx_pool &pool)
    : protocol_events(network, channel, NAME),
      blockchain_(blockchain),
      pool_(pool),

      // TODO: move fee filter to a derived class protocol_tx_out_70013.
      minimum_fee_(0),

      // TODO: move relay to a derived class protocol_tx_out_70001.
      relay_to_peer_(peer_version().relay),
      CONSTRUCT_TRACK(protocol_tx_out)
{
}

protocol_tx_out::ptr protocol_tx_out::do_subscribe()
{
    SUBSCRIBE2(memory_pool, handle_receive_memory_pool, _1, _2);
    SUBSCRIBE2(fee_filter, handle_receive_fee_filter, _1, _2);
    SUBSCRIBE2(get_data, handle_receive_get_data, _1, _2);
    protocol_events::start(BIND1(handle_stop, _1));
    return std::dynamic_pointer_cast<protocol_tx_out>(protocol::shared_from_this());
}

// TODO: move not_found to derived class protocol_tx_out_70001.

// Start.
//-----------------------------------------------------------------------------

void protocol_tx_out::start()
{
    // TODO: move relay to a derived class protocol_tx_out_70001.
    // Prior to this level transaction relay is not configurable.
    if (relay_to_peer_)
    {
        // Subscribe to transaction pool notifications and relay txs.
        pool_.subscribe_transaction(BIND3(handle_floated, _1, _2, _3));
        if (channel_stopped())
        {
            pool_.fired();
        }
    }

    // TODO: move fee filter to a derived class protocol_tx_out_70013.
    // Filter announcements by fee if set.
}

// Receive send_headers.
//-----------------------------------------------------------------------------

// TODO: move fee_filters to a derived class protocol_tx_out_70013.
bool protocol_tx_out::handle_receive_fee_filter(const code &ec,
                                                         fee_filter_ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::trace(LOG_NODE)
            << "Failure getting " << message->command << " from ["
            << authority() << "] " << ec.message();
        stop(ec);
        return false;
    }

    // TODO: move fee filter to a derived class protocol_tx_out_70013.
    // Transaction annoucements will be filtered by fee amount.
    minimum_fee_.store(message->minimum_fee);

    // The fee filter may be adjusted.
    return true;
}

// Receive mempool sequence.
//-----------------------------------------------------------------------------

bool protocol_tx_out::handle_receive_memory_pool(const code &ec,
                                                          memory_pool_ptr)
{
    if (stopped())
    {
        return false;
    }

    if (ec)
    {
        return false;
    }

    auto self = shared_from_this();
    pool_.fetch([this, self](const code &ec, const std::vector<transaction_ptr> &txs) {
        if (stopped() || ec)
        {
            log::debug(LOG_NODE) << "pool fetch transaction failed," << ec.message();
            return;
        }

        if (txs.empty())
        {
            return;
        }
        std::vector<hash_digest> hashes;
        hashes.reserve(txs.size());
        for (auto &t : txs)
        {
            hashes.push_back(t->hash());
        }
        send<protocol_tx_out>(inventory{hashes, inventory::type_id::transaction}, &protocol_tx_out::handle_send, _1, inventory::command);
    });
    return false;
}

// Receive get_data sequence.
//-----------------------------------------------------------------------------

bool protocol_tx_out::handle_receive_get_data(const code &ec,
                                                       get_data_ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::trace(LOG_NODE)
            << "Failure getting inventory from [" << authority() << "] "
            << ec.message();
        stop(ec);
        return false;
    }

    //    if (message->inventories.size() > 50000)
    //    {
    //        return ! misbehaving(20);
    //    }

    // TODO: these must return message objects or be copied!
    // Ignore non-transaction inventory requests in this protocol.
    for (const auto &inv : message->inventories)
        if (inv.type == inventory::type_id::transaction)
        {
            auto pThis = shared_from_this();
            pool_.fetch(inv.hash, [this, &inv, pThis](const code &ec, transaction_ptr tx) {
                auto t = tx ? *tx : chain::transaction{};
                send_transaction(ec, t, inv.hash);
                if (ec)
                {
                    blockchain_.fetch_transaction(inv.hash,
                                                  BIND3(send_transaction, _1, _2, inv.hash));
                }
            });
        }

    return true;
}

void protocol_tx_out::send_transaction(const code &ec,
                                                const chain::transaction &transaction, const hash_digest &hash)
{
    if (stopped() || ec == (code)error::service_stopped)
        return;

    if (ec == (code)error::not_found)
    {
        log::trace(LOG_NODE)
            << "Transaction requested by [" << authority() << "] not found.";

        //        const not_found reply{ { inventory::type_id::transaction, hash } };
        //        SEND2(reply, handle_send, _1, reply.command);
        return;
    }

    if (ec)
    {
        log::error(LOG_NODE)
            << "Internal failure locating trnsaction requested by ["
            << authority() << "] " << ec.message();
        stop(ec);
        return;
    }

    log::trace(LOG_NODE) << "send transaction " << encode_hash(transaction.hash()) << ", to " << authority();

    // TODO: eliminate copy.
    SEND2(tx_message(transaction), handle_send, _1,
          tx_message::command);
}

// Subscription.
//-----------------------------------------------------------------------------

bool protocol_tx_out::handle_floated(const code &ec,
                                              const index_list &unconfirmed, transaction_ptr message)
{
    if (stopped() || ec == (code)error::service_stopped)
        return false;

    if (ec == (code)error::mock)
        return true;

    if (ec)
    {
        log::error(LOG_NODE)
            << "Failure handling transaction float: " << ec.message();
        stop(ec);
        return false;
    }

    // TODO: move fee filter to a derived class protocol_tx_out_70013.
    // TODO: implement fee computation.
    const uint64_t fee = 0;

    // Transactions are discovered and announced individually.
    if (message->originator() != nonce() && fee >= minimum_fee_.load())
    {
        static const auto id = inventory::type_id::transaction;
        const inventory announcement{{id, message->hash()}};
        log::trace(LOG_NODE) << "handle floated send transaction hash," << encode_hash(message->hash());
        SEND2(announcement, handle_send, _1, announcement.command);
    }

    return true;
}

void protocol_tx_out::handle_stop(const code &)
{
    log::trace(LOG_NETWORK)
        << "Stopped transaction_out protocol";
    pool_.fired();
}

} // namespace node
} // namespace libbitcoin
