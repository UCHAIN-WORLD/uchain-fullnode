/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-consensus.
 *
 * UChain-consensus is free software: you can redistribute it and/or
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

#ifndef UC_CONSENSUS_MINER_HPP
#define UC_CONSENSUS_MINER_HPP

#include <vector>
#include <boost/thread.hpp>

#include "UChain/blockchain/block_chain_impl.hpp"
#include "UChain/blockchain/transaction_pool.hpp"
#include "UChain/bitcoin/chain/block.hpp"
#include "UChain/bitcoin/chain/input.hpp"
#include <UChain/bitcoin/wallet/ec_public.hpp>
#include <UChain/blockchain/settings.hpp>
#include <UChain/explorer/config/ec_private.hpp>
#include <UChain/explorer/config/hashtype.hpp>
#include <UChain/explorer/config/script.hpp>
#include <mutex>

namespace libbitcoin {
namespace node {
class p2p_node;
}
}

namespace libbitcoin {
namespace consensus {

BC_CONSTEXPR unsigned int min_tx_fee_per_kb = 1000;
BC_CONSTEXPR unsigned int median_time_span = 11;
BC_CONSTEXPR uint32_t version = 1;

//extern int bucket_size;
extern vector<uint64_t> lock_heights;

class miner
{
public:
    typedef message::block_message block;
    typedef std::shared_ptr<message::block_message> block_ptr;
    typedef chain::header header;
    typedef chain::transaction transaction;
    typedef message::transaction_message::ptr transaction_ptr;
    typedef blockchain::block_chain_impl block_chain_impl;
    typedef blockchain::transaction_pool transaction_pool;
    typedef libbitcoin::node::p2p_node p2p_node;

    // prev_output_point -> (prev_block_height, prev_output)
    typedef std::unordered_map<chain::point, std::pair<uint64_t, chain::output>> previous_out_map_t;

    // tx_hash -> tx_fee
    typedef std::unordered_map<hash_digest, uint64_t> tx_fee_map_t;

    miner(p2p_node& node);
    ~miner();

    enum state
    {
        init_,
        exit_,
        creating_block_
    };

    bool start(const wallet::payment_address& pay_address, uint16_t number = 0);
    bool start(const std::string& pay_public_key, uint16_t number = 0);
    bool stop();
    static block_ptr create_genesis_block(bool is_mainnet);
    bool script_hash_signature_operations_count(size_t &count, const chain::input::list& inputs,
        vector<transaction_ptr>& transactions);
    bool script_hash_signature_operations_count(size_t &count, const chain::input& input,
        vector<transaction_ptr>& transactions);
    transaction_ptr create_coinbase_tx(const wallet::payment_address& pay_addres,
        uint64_t value,uint64_t block_height);
    transaction_ptr create_lock_coinbase_tx(const wallet::payment_address& pay_addres,
        uint64_t value, uint64_t block_height, int lock_height, uint32_t reward_lock_time);

    block_ptr get_block(bool is_force_create_block = false);
    //bool get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary);
    /*bool put_result(const std::string& nonce, const std::string& mix_hash,
        const std::string& header_hash, const uint64_t &nounce_mask);*/
    //bool set_miner_public_key(const string& public_key);
    uint64_t fetch_utxo(const transaction_ptr& ptx,const wallet::payment_address& address);
    bool get_spendable_output(chain::output& output, const chain::history& row, uint64_t height);
    bool set_miner_payment_address(const wallet::payment_address& address);
    const std::string get_miner_address();
    bool set_miner_pri_key(const std::string& pri_key);
    //void set_user(const std::string& name, const std::string& passwd);
    void get_state(uint64_t &height,  uint32_t &miners,/*uint64_t &rate, string& difficulty,*/ bool& is_mining);
    vector<std::string>& get_miners();
    bool is_creating_block() const;
    
    bool get_block_header(chain::header& block_header, const string& para);

    static int get_lock_heights_index(uint64_t height);
    static uint64_t calculate_block_subsidy(uint64_t height, bool is_testnet);
    static uint64_t calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num);

private:
    void work(const wallet::payment_address pay_address);
    int get_mine_index(const wallet::payment_address& pay_address) const;
    block_ptr create_new_block(const wallet::payment_address& pay_addres,uint64_t current_block_height = max_uint64);
    unsigned int get_adjust_time(uint64_t height) const;
    unsigned int get_median_time_past(uint64_t height) const;
    bool get_transaction(std::vector<transaction_ptr>&, previous_out_map_t&, tx_fee_map_t&) const;
    uint64_t store_block(block_ptr block);
    uint64_t get_height() const;
    bool get_input_ucn(const transaction&, const std::vector<transaction_ptr>&, uint64_t&, previous_out_map_t&) const ;
    bool is_stop_miner(uint64_t block_height) const;

private:
    p2p_node& node_;
    std::shared_ptr<boost::thread> thread_;
    mutable state state_;
    uint16_t new_block_number_;
    uint16_t new_block_limit_;

    block_ptr new_block_;
    wallet::payment_address pay_address_;
    const blockchain::settings& setting_;
    std::string pri_key;
    std::string name_;
    std::string passwd_;

    vector<std::string> mine_address_list = {
                                                /*"UPqb2AfKPpfqFoxAaujmH7Ay3CiGQgue7h",
                                                "UeBhVsr28ovcBS5DjxqXtHa3ueCP6o2FQi",
                                                "UcuW7wVu198Nuzok8eeMDUNEZQoGqQRRz5"*/
                                                "UXFQvGKWh8GzEtV1RNw2Vo1abnynPy58u1"
                                            };

};

}
}

#endif
