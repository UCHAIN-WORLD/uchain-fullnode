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
#ifndef UC_BLOCKCHAIN_VALIDATE_TRANSACTION_HPP
#define UC_BLOCKCHAIN_VALIDATE_TRANSACTION_HPP

#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/blockchain/define.hpp>
#include <UChain/blockchain/tx_pool.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>

namespace libbitcoin
{
namespace blockchain
{

class validate_block;

/// This class is not thread safe.
/// This is a utility for tx_pool::validate and validate_block.
class BCB_API validate_tx_engine
    : public enable_shared_from_base<validate_tx_engine>
{
  public:
    typedef std::shared_ptr<validate_tx_engine> ptr;
    typedef message::transaction_message::ptr transaction_ptr;
    typedef std::function<void(const code &, transaction_ptr,
                               chain::point::indexes)>
        validate_handler;

    validate_tx_engine(block_chain &chain, const chain::transaction &tx,
                         const tx_pool &pool, dispatcher &dispatch);

    validate_tx_engine(block_chain &chain, const chain::transaction &tx,
                         const validate_block &validate_block);

    void start(validate_handler handler);

    static bool check_consensus(const chain::script &prevout_script,
                                const chain::transaction &current_tx, size_t input_index,
                                uint32_t flags);

    code check_transaction_connect_input(size_t last_height);
    code check_transaction() const;
    code check_transaction_basic() const;
    code check_token_issue_transaction() const;
    code check_token_cert_transaction() const;
    code check_secondaryissue_transaction() const;
    code check_candidate_transaction() const;
    code check_uid_transaction() const;
    bool connect_uid_input(const uid &info) const;
    bool is_uid_match_address_in_orphan_chain(const std::string &symbol, const std::string &address) const;
    bool is_uid_in_orphan_chain(const std::string &uid) const;
    code check_asset_to_uid(const output &output) const;
    code connect_asset_from_uid(const output &output) const;

    bool connect_input(const chain::transaction &previous_tx, size_t parent_height);

    static bool tally_fees(blockchain::block_chain_impl &chain,
                           const chain::transaction &tx, uint64_t value_in, uint64_t &fees);
    static bool check_special_fees(bool is_testnet, const chain::transaction &tx, uint64_t fees);

    bool check_token_amount(const transaction &tx) const;
    bool check_token_symbol(const transaction &tx) const;
    bool check_token_certs(const transaction &tx) const;
    bool check_candidate(const transaction &tx) const;
    bool check_address_registered_uid(const std::string &address) const;

    //check input uid match output uid
    bool check_uid_symbol_match(const transaction &tx) const;

    static bool is_uid_feature_activated(blockchain::block_chain_impl &chain);

    bool get_previous_tx(chain::transaction &prev_tx, uint64_t &prev_height, const chain::input &) const;

    transaction &get_tx() { return *tx_; }
    const transaction &get_tx() const { return *tx_; }
    blockchain::block_chain_impl &get_blockchain() { return blockchain_; }
    const blockchain::block_chain_impl &get_blockchain() const { return blockchain_; }

  private:
    code basic_checks() const;
    bool is_standard() const;
    void handle_duplicate_check(const code &ec);
    void reset(size_t last_height);

    // Last height used for checking coinbase maturity.
    void set_last_height(const code &ec, size_t last_height);

    // Begin looping through the inputs, fetching the previous tx
    void next_previous_transaction();
    void previous_tx_index(const code &ec, size_t parent_height);

    // If previous_tx_index uidn't find it then check in pool instead
    void search_pool_previous_tx();
    void handle_previous_tx(const code &ec,
                            const chain::transaction &previous_tx, size_t parent_height);

    // After running connect_input, we check whether this validated previous
    // output was not already spent by another input in the blockchain.
    // is_spent() earlier already checked in the pool.
    void check_double_spend(const code &ec, const chain::input_point &point);
    void check_fees() const;
    code check_tx_connect_input() const;
    code check_tx_connect_output() const;
    bool check_uid_exist(const std::string &uid) const;
    bool check_token_exist(const std::string &symbol) const;
    bool check_token_cert_exist(const std::string &cert, token_cert_type cert_type) const;
    bool check_candidate_exist(const std::string &candidate) const;

    block_chain_impl &blockchain_;
    const transaction_ptr tx_;
    const tx_pool *const pool_;
    dispatcher *const dispatch_;
    const validate_block *const validate_block_;

    const hash_digest tx_hash_;
    size_t last_block_height_;
    uint64_t value_in_;
    uint64_t token_amount_in_;
    std::vector<token_cert_type> token_certs_in_;
    std::string old_symbol_in_;      // used for check same token/uid/candidate symbol in previous outputs
    std::string old_cert_symbol_in_; // used for check same cert symbol in previous outputs
    uint32_t current_input_;
    chain::point::indexes unconfirmed_;
    validate_handler handle_validate_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
