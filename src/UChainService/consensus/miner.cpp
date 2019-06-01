/**
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
#include <UChainService/consensus/miner.hpp>
#include <UChain/blockchain/block_chain.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChain/blockchain/validate_block.hpp>
#include <UChain/node/p2p_node.hpp>

#include <algorithm>
#include <functional>
#include <system_error>
#include <boost/thread.hpp>
//#include <UChainService/consensus/miner/MinerAux.h>
//#include <UChainService/consensus/libdevcore/BasicType.h>
#include <UChain/coin/chain/script/operation.hpp>
#include <UChain/coin/config/hash160.hpp>
#include <UChain/coin/wallet/ec_public.hpp>
#include <UChain/coin/constants.hpp>
#include <UChain/blockchain/validate_block.hpp>
#include <UChain/blockchain/validate_transaction.hpp>
#include <UChain/coin/utility/time.hpp>
#define LOG_HEADER "consensus"
using namespace std;
using namespace std::this_thread;

namespace libbitcoin
{
namespace consensus
{

static BC_CONSTEXPR unsigned int min_tx_fee = 100000;

// tuples: (priority, fee_per_kb, fee, transaction_ptr)
typedef boost::tuple<double, double, uint64_t, miner::transaction_ptr> transaction_priority;

namespace
{
// fee : per kb
bool sort_by_fee_per_kb(const transaction_priority &a, const transaction_priority &b)
{
    if (a.get<1>() == b.get<1>())
        return a.get<0>() < b.get<0>();
    return a.get<1>() < b.get<1>();
};

// priority : coin age
bool sort_by_priority(const transaction_priority &a, const transaction_priority &b)
{
    if (a.get<0>() == b.get<0>())
        return a.get<1>() < b.get<1>();
    return a.get<0>() < b.get<0>();
};
} // end of anonymous namespace

miner::miner(p2p_node &node)
    : node_(node), state_(state::init_), new_block_number_(0), new_block_limit_(0), createblockms_(0), setting_(node_.chain_impl().chain_settings())
{
    /*if (setting_.use_testnet_rules) {
        bc::HeaderAux::set_as_testnet();
    }*/
    mine_candidate_list = {};
    mine_address_list = {};
}

miner::~miner()
{
    stop();
}

bool miner::get_input_ucn(const transaction &tx, const std::vector<transaction_ptr> &transactions,
                          uint64_t &total_inputs, previous_out_map_t &previous_out_map) const
{
    total_inputs = 0;
    block_chain_impl &block_chain = node_.chain_impl();
    for (auto &input : tx.inputs)
    {
        transaction prev_tx;
        uint64_t prev_height = 0;
        uint64_t input_value = 0;
        if (block_chain.get_transaction(prev_tx, prev_height, input.previous_output.hash))
        {
            input_value = prev_tx.outputs[input.previous_output.index].value;
            previous_out_map[input.previous_output] =
                std::make_pair(prev_height, prev_tx.outputs[input.previous_output.index]);
        }
        else
        {
            const hash_digest &hash = input.previous_output.hash;
            const auto found = [&hash](const transaction_ptr &entry) {
                return entry->hash() == hash;
            };
            auto it = std::find_if(transactions.begin(), transactions.end(), found);
            if (it != transactions.end())
            {
                input_value = (*it)->outputs[input.previous_output.index].value;
                previous_out_map[input.previous_output] =
                    std::make_pair(max_uint64, (*it)->outputs[input.previous_output.index]);
            }
            else
            {
#ifdef UC_DEBUG
                log::debug(LOG_HEADER) << "previous transaction not ready: " << encode_hash(hash);
#endif
                return false;
            }
        }

        total_inputs += input_value;
    }

    return true;
}

bool miner::get_transaction(std::vector<transaction_ptr> &transactions,
                            previous_out_map_t &previous_out_map, tx_fee_map_t &tx_fee_map) const
{
    boost::mutex mutex;
    mutex.lock();
    auto f = [&transactions, &mutex](const error_code &code, const vector<transaction_ptr> &transactions_) -> void {
        transactions = transactions_;
        mutex.unlock();
    };
    node_.pool().fetch(f);

    boost::unique_lock<boost::mutex> lock(mutex);

    if (transactions.empty() == false)
    {
        set<hash_digest> sets;
        for (auto i = transactions.begin(); i != transactions.end();)
        {
            auto &tx = **i;
            auto hash = tx.hash();

            uint64_t total_input_value = 0;
            bool ready = get_input_ucn(tx, transactions, total_input_value, previous_out_map);
            if (!ready)
            {
                // erase tx but not delete it from pool if parent tx is not ready
                i = transactions.erase(i);
                break;
            }

            uint64_t total_output_value = tx.total_output_value();
            uint64_t fee = total_input_value - total_output_value;

            // check fees
            if (fee < min_tx_fee || !blockchain::validate_transaction::check_special_fees(setting_.use_testnet_rules, tx, fee))
            {
                i = transactions.erase(i);
                // delete it from pool if not enough fee
                node_.pool().delete_tx(hash);
                break;
            }

            auto transaction_is_ok = true;
            for (auto &output : tx.outputs)
            {
                if (tx.version >= transaction_version::check_output_script && output.script.pattern() == script_pattern::non_standard)
                {
#ifdef UC_DEBUG
                    log::error(LOG_HEADER) << "transaction output script error! tx:" << tx.to_string(1);
#endif
                    node_.pool().delete_tx(hash);
                    transaction_is_ok = false;
                    break;
                }
            }

            if (transaction_is_ok && (sets.find(hash) == sets.end()))
            {
                tx_fee_map[hash] = fee;
                sets.insert(hash);
                ++i;
            }
            else
            {
                i = transactions.erase(i);
            }
        }
    }
    return transactions.empty() == false;
}

bool miner::script_hash_signature_operations_count(size_t &count, const chain::input &input, vector<transaction_ptr> &transactions)
{
    const auto &previous_output = input.previous_output;
    transaction previous_tx;
    boost::uint64_t h;
    if (node_.chain_impl().get_transaction(previous_tx, h, previous_output.hash) == false)
    {
        bool found = false;
        for (auto &tx : transactions)
        {
            if (previous_output.hash == tx->hash())
            {
                previous_tx = *tx;
                found = true;
                break;
            }
        }

        if (found == false)
            return false;
    }

    const auto &previous_tx_out = previous_tx.outputs[previous_output.index];
    return blockchain::validate_block::script_hash_signature_operations_count(count, previous_tx_out.script, input.script);
}

bool miner::script_hash_signature_operations_count(
    size_t &count, const chain::input::list &inputs, vector<transaction_ptr> &transactions)
{
    count = 0;
    for (const auto &input : inputs)
    {
        size_t c = 0;
        if (script_hash_signature_operations_count(c, input, transactions) == false)
            return false;
        count += c;
    }
    return true;
}

#define VALUE(a) (a < 'a' ? (a - '0') : (a - 'a' + 10))
std::string transfer_public_key(const string &key)
{
    string pub_key;
    for (auto i = key.begin(); i != key.end(); i += 2)
    {
        unsigned char a = 0;
        a = (VALUE(*i) << 4) + VALUE(*(i + 1));
        pub_key.push_back(a);
    }

    return pub_key;
}

miner::block_ptr miner::create_genesis_block(bool is_mainnet)
{
    string text;
    if (is_mainnet)
    {
        text = "2018-02-14 00:00:00 UC start running mainnet.";
    }
    else
    {
        text = "2018-10-18 14:16:55 UC start running testnet.";
    }

    block_ptr pblock = make_shared<block>();

    // Create coinbase tx
    transaction tx_new;
    tx_new.inputs.resize(1);
    tx_new.inputs[0].previous_output = {null_hash, max_uint32};
    tx_new.inputs[0].script.operations = {{chain::opcode::raw_data, {text.begin(), text.end()}}};
    tx_new.outputs.resize(1);

    // init for testnet/mainnet
    bc::wallet::payment_address genesis_address(get_foundation_address(!is_mainnet));
    tx_new.outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(genesis_address));
    pblock->header.timestamp = 1550073600;

    tx_new.outputs[0].value = total_reward * coin_price();

    // Add our coinbase tx as first transaction
    pblock->transactions.push_back(tx_new);

    // Fill in header
    pblock->header.previous_block_hash = null_hash;
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);
    pblock->header.transaction_count = 1;
    pblock->header.version = 1;
    /*pblock->header.bits = 1;
    pblock->header.nonce = 0;*/
    pblock->header.number = 0;
    //pblock->header.mixhash = 0;

    return pblock;
}

miner::transaction_ptr miner::create_coinbase_tx(
    const bc::wallet::payment_address &pay_address, uint64_t value, uint64_t block_height)
{
    transaction_ptr ptransaction = make_shared<message::transaction_message>();

    //auto start =unix_millisecond();
    const uint64_t unspent_token = fetch_utxo(ptransaction, pay_address);
    //log::info(LOG_HEADER) << "solo miner fetch_utxo for " << unix_millisecond() - start << " ms";
    if (!unspent_token)
    {
        ptransaction->version = version;
        ptransaction->inputs.resize(1);
        ptransaction->inputs[0].previous_output = {null_hash, max_uint32};
        script_number number(block_height);
        ptransaction->inputs[0].script.operations.push_back({chain::opcode::special, number.data()});
    }
    else
    {
        ptransaction->version = transaction_version::check_output_script;
    }
    ptransaction->locktime = 0;

    ptransaction->outputs.resize(1);

    ptransaction->outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(pay_address));

    auto transfer = chain::token_transfer(UC_BLOCK_TOKEN_SYMBOL, unspent_token + 1);
    auto ass = token(TOKEN_TRANSFERABLE_TYPE, transfer);

    ptransaction->outputs[0].value = 0; //1 block
    ptransaction->outputs[0].attach_data = asset(UC_TOKEN_TYPE, 1, ass);

    return ptransaction;
}

miner::transaction_ptr miner::create_lock_coinbase_tx(
    const bc::wallet::payment_address &pay_address, uint64_t value,
    uint64_t block_height, int lock_height, uint32_t reward_lock_time)
{

    if (!(lock_height > 0 && value > 0))
    {
        return nullptr;
    }

    transaction_ptr ptransaction = make_shared<message::transaction_message>();

    ptransaction->version = version;
    ptransaction->inputs.resize(1);
    ptransaction->inputs[0].previous_output = {null_hash, max_uint32};
    script_number number(block_height);
    ptransaction->inputs[0].script.operations.push_back({chain::opcode::special, number.data()});

    ptransaction->outputs.resize(1);

    ptransaction->locktime = reward_lock_time;

    ptransaction->outputs[0].script.operations = chain::operation::to_pay_key_hash_with_lock_height_pattern(short_hash(pay_address), lock_height);

    ptransaction->outputs[0].value = value;

    return ptransaction;
}

uint64_t miner::fetch_utxo(const transaction_ptr &ptx, const bc::wallet::payment_address &address)
{
    block_chain_impl &block_chain = node_.chain_impl();
    uint64_t height = 0, index = 0, unspent_ucn{0}, unspent_token{0};
    block_chain.get_last_height(height);

    auto fromheight = height > mine_fetch_utxo_height_windows ? height - mine_fetch_utxo_height_windows : 0;
    auto &&rows = block_chain.get_address_history(address, true, fromheight);
    if (!rows.size())
    {
        return false;
    }

    for (auto &row : rows)
    {
        chain::output output;
        chain::input input;
        if (!get_spendable_output(output, row, height))
        {
            continue;
        }

        if (output.get_script_address() != address.encoded())
        {
            continue;
        }

        auto token_symbol = output.get_token_symbol();

        if (!(output.is_token_transfer() && token_symbol == UC_BLOCK_TOKEN_SYMBOL))
        {
            continue;
        }

        //auto ucn_amount = row.value;
        auto token_total_amount = output.get_token_amount();
        auto cert_type = output.get_token_cert_type();

        //BITCOIN_ASSERT(ucn_amount == 0);
        BITCOIN_ASSERT(cert_type == token_cert_ns::none);
        if (token_total_amount == 0)
            continue;

        if (index && row.output.hash == ptx->inputs[index - 1].previous_output.hash && row.output.index == ptx->inputs[index - 1].previous_output.index)
            continue;
        input.previous_output = {row.output.hash, row.output.index};
        //input.script = output.script;
        input.sequence = max_input_sequence;
        ptx->inputs.push_back(input);
        //spend UTXO
        bc::chain::script ss;
        bc::explorer::config::hashtype sign_type;
        uint8_t hash_type = (signature_hash_algorithm)sign_type;
        bc::explorer::config::ec_private config_private_key(pri_key);
        const ec_secret &private_key = config_private_key;

        bc::explorer::config::script config_contract(output.script);
        const bc::chain::script &contract = config_contract;

        // gen sign
        bc::endorsement endorse;
        if (!bc::chain::script::create_endorsement(endorse, private_key,
                                                   contract, *ptx, index, hash_type))
        {
            return false;
        }

        // do script
        bc::wallet::ec_private ec_private_key(private_key, 0u, true);
        auto &&public_key = ec_private_key.to_public();
        data_chunk public_key_data;
        public_key.to_data(public_key_data);

        ss.operations.push_back({bc::chain::opcode::special, endorse});
        ss.operations.push_back({bc::chain::opcode::special, public_key_data});

        // if pre-output script is deposit tx.
        if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
        {
            uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                contract.operations);
            ss.operations.push_back({bc::chain::opcode::special, script_number(lock_height).data()});
        }
        ptx->inputs[index++].script = ss;
        // unspent_ucn += row.value;
        unspent_token += output.get_token_amount();
        break;
    }
    rows.clear();
    return unspent_token;
}

bool miner::get_spendable_output(chain::output &output, const chain::history &row, uint64_t height)
{
    if (row.spend.hash != null_hash)
    {
        return false;
    }

    block_chain_impl &block_chain = node_.chain_impl();
    chain::transaction tx_temp;
    uint64_t tx_height;
    if (!block_chain.get_transaction(row.output.hash, tx_temp, tx_height))
    {
        return false;
    }

    BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
    output = tx_temp.outputs.at(row.output.index);

    if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations))
    {
        if (row.output_height == 0)
        {
            // deposit utxo in transaction pool
            return false;
        }
        else
        {
            // deposit utxo in block
            auto lock_height = chain::operation::
                get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
            if ((row.output_height + lock_height) > height)
            {
                // utxo already in block but deposit not expire
                return false;
            }
        }
    }
    else if (tx_temp.is_coinbase())
    { // incase readd deposit
        // coin base ucn maturity ucn check
        // coinbase_maturity ucn check
        if (row.output_height == 0 /*|| (row.output_height + coinbase_maturity) > height*/)
        {
            return false;
        }
    }

    return true;
}

//int bucket_size = mine_block_produce_minsecons000;
vector<uint64_t> lock_heights = {1728000, 7776000, 20736000, 41472000, 93312000};
vector<uint64_t> coinage_rewards = {300000, 1990000, 5200000, 10000000, 24000000};

int miner::get_lock_heights_index(uint64_t height)
{
    int ret = -1;
    auto it = find(lock_heights.begin(), lock_heights.end(), height);
    if (it != lock_heights.end())
    {
        ret = it - lock_heights.begin();
    }
    return ret;
}

uint64_t miner::calculate_block_subsidy(uint64_t block_height, bool is_testnet)
{
    return 0; //min_tx_fee;///uint64_t(3 * coin_price() * pow(0.95, block_height / bucket_size));
}

uint64_t miner::calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num)
{
    int lock_heights_index = get_lock_heights_index(lcok_heights);
    if (lock_heights_index >= 0)
    {
        double n = ((double)coinage_rewards[lock_heights_index]) / coin_price();
        return (uint64_t)(n * num);
    }
    return 0;
}

struct transaction_dependent
{
    std::shared_ptr<hash_digest> hash;
    unsigned short dpendens;
    bool is_need_process;
    transaction_priority transaction;

    transaction_dependent() : dpendens(0), is_need_process(false) {}
    transaction_dependent(const hash_digest &_hash, unsigned short _dpendens, bool _is_need_process)
        : dpendens(_dpendens), is_need_process(_is_need_process) { hash = make_shared<hash_digest>(_hash); }
};

miner::block_ptr miner::create_new_block(const bc::wallet::payment_address &pay_address, uint64_t current_block_height)
{
    block_ptr pblock;
    vector<transaction_ptr> transactions;
    map<hash_digest, transaction_dependent> transaction_dependents;
    previous_out_map_t previous_out_map;
    tx_fee_map_t tx_fee_map;
    get_transaction(transactions, previous_out_map, tx_fee_map);

    vector<transaction_priority> transaction_prioritys;
    block_chain_impl &block_chain = node_.chain_impl();

    header prev_header;
    if (current_block_height == max_uint64)
    {
        if (!block_chain.get_last_height(current_block_height) || !block_chain.get_header(prev_header, current_block_height))
        {
            log::warning(LOG_HEADER) << "get_last_height or get_header fail. current_block_height:" << current_block_height;
            return pblock;
        }
    }
    else if (!block_chain.get_header(prev_header, current_block_height))
    {
        log::warning(LOG_HEADER) << "get_last_height or get_header fail. current_block_height:" << current_block_height;
        return pblock;
    }
    else
    {
        pblock = make_shared<block>();
    }

    // Create coinbase tx
    pblock->transactions.push_back(*create_coinbase_tx(pay_address, 0, current_block_height + 1));

    // Largest block you're willing to create:
    unsigned int block_max_size = blockchain::max_block_size / 2;
    // Limit to betweeen 1K and max_block_size-1K for sanity:
    block_max_size = max((unsigned int)1000, min((unsigned int)(blockchain::max_block_size - 1000), block_max_size));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int block_priority_size = 27000;
    block_priority_size = min(block_max_size, block_priority_size);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int block_min_size = 0;
    block_min_size = min(block_max_size, block_min_size);

    uint64_t total_fee = 0;
    unsigned int block_size = 0;
    unsigned int total_tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*pblock->transactions.begin());
    for (auto tx : transactions)
    {
        auto tx_hash = tx->hash();
        double priority = 0;
        for (const auto &input : tx->inputs)
        {
            auto prev_pair = previous_out_map[input.previous_output];
            uint64_t prev_height = prev_pair.first;
            const auto &prev_output = prev_pair.second;

            if (prev_height != max_uint64)
            {
                uint64_t input_value = prev_output.value;
                priority += (double)input_value * (current_block_height - prev_height + 1);
            }
            else
            {
                transaction_dependents[input.previous_output.hash].hash = make_shared<hash_digest>(tx_hash);
                transaction_dependents[tx_hash].dpendens++;
            }
        }

        uint64_t serialized_size = tx->serialized_size(0);

        // Priority is sum(valuein * age) / txsize
        priority /= serialized_size;

        // This is a more accurate fee-per-kilobyte than is used by the client code, because the
        // client code rounds up the size to the nearest 1K. That's good, because it gives an
        // incentive to create smaller transactions.
        auto tx_fee = tx_fee_map[tx_hash];
        double fee_per_kb = double(tx_fee) / (double(serialized_size) / 1000.0);
        transaction_prioritys.push_back(transaction_priority(priority, fee_per_kb, tx_fee, tx));
    }

    vector<transaction_ptr> blocked_transactions;
    auto sort_func = sort_by_fee_per_kb;
    bool is_resort = false;
    make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);

    transaction_priority *next_transaction_priority = NULL;
    uint32_t reward_lock_time = current_block_height - 1;
    while (!transaction_prioritys.empty() || next_transaction_priority)
    {
        transaction_priority temp_priority;
        if (next_transaction_priority)
        {
            temp_priority = *next_transaction_priority;
        }
        else
        {
            temp_priority = transaction_prioritys.front();
        }

        double priority = temp_priority.get<0>();
        double fee_per_kb = temp_priority.get<1>();
        uint64_t fee = temp_priority.get<2>();
        transaction_ptr ptx = temp_priority.get<3>();

        if (next_transaction_priority)
        {
            next_transaction_priority = NULL;
        }
        else
        {
            pop_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);
            transaction_prioritys.pop_back();
        }

        hash_digest h = ptx->hash();
        if (transaction_dependents[h].dpendens != 0)
        {
            transaction_dependents[h].transaction = temp_priority;
            transaction_dependents[h].is_need_process = true;
            continue;
        }

        // Size limits
        uint64_t serialized_size = ptx->serialized_size(1);
        vector<transaction_ptr> coinage_reward_coinbases;
        transaction_ptr coinage_reward_coinbase;
        for (const auto &output : ptx->outputs)
        {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations))
            {
                int lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                coinage_reward_coinbase = create_lock_coinbase_tx(ptx->has_candidate_register() ? bc::wallet::payment_address(bc::get_developer_community_address(block_chain.chain_settings().use_testnet_rules)) : bc::wallet::payment_address::extract(ptx->outputs[0].script),
                                                                  calculate_lockblock_reward(lock_height, output.value),
                                                                  current_block_height + 1, lock_height, reward_lock_time);

                if (!coinage_reward_coinbase)
                {
                    continue;
                }
                unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*coinage_reward_coinbase);
                if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops)
                {
                    continue;
                }

                total_tx_sig_length += tx_sig_length;
                serialized_size += coinage_reward_coinbase->serialized_size(1);
                coinage_reward_coinbases.push_back(coinage_reward_coinbase);
                --reward_lock_time;
            }
        }

        if (block_size + serialized_size >= block_max_size)
            continue;

        // Legacy limits on sigOps:
        unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*ptx);
        if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops)
            continue;

        // Skip free transactions if we're past the minimum block size:
        if (is_resort && (fee_per_kb < min_tx_fee_per_kb) && (block_size + serialized_size >= block_min_size))
            break;

        // Prioritize by fee once past the priority size or we run out of high-priority
        // transactions:
        if (is_resort == false &&
            ((block_size + serialized_size >= block_priority_size) || (priority < coin_price() * 144 / 250)))
        {
            sort_func = sort_by_priority;
            is_resort = true;
            make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);
        }

        size_t c;
        if (!miner::script_hash_signature_operations_count(c, ptx->inputs, transactions) && total_tx_sig_length + tx_sig_length + c >= blockchain::max_block_script_sigops)
            continue;
        tx_sig_length += c;

        blocked_transactions.push_back(ptx);
        for (auto &i : coinage_reward_coinbases)
        {
            pblock->transactions.push_back(*i);
        }

        block_size += serialized_size;
        total_tx_sig_length += tx_sig_length;
        total_fee += fee;

        if (transaction_dependents[h].hash)
        {
            transaction_dependent &d = transaction_dependents[*transaction_dependents[h].hash];
            if (--d.dpendens == 0 && d.is_need_process)
            {
                next_transaction_priority = &d.transaction;
            }
        }
    }

    for (auto i : blocked_transactions)
    {
        pblock->transactions.push_back(*i);
    }

    uint64_t total_value =
        total_fee + calculate_block_subsidy(current_block_height + 1, setting_.use_testnet_rules);

    if (total_value > 0)
    {
        pblock->transactions[0].outputs.resize(2);
        pblock->transactions[0].outputs[1].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(pay_address));
        pblock->transactions[0].outputs[1].value = total_value;
    }

    // Fill in header
    pblock->header.number = current_block_height + 1;
    pblock->header.transaction_count = pblock->transactions.size();
    pblock->header.previous_block_hash = prev_header.hash();
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);
    pblock->header.timestamp = get_adjust_time(pblock->header.number);
    pblock->header.version = version;
    /*pblock->header.bits = HeaderAux::calculateDifficulty(pblock->header, prev_header);
    pblock->header.nonce = 0;
    pblock->header.mixhash = 0;*/

    return pblock;
}

unsigned int miner::get_adjust_time(uint64_t height) const
{
    typedef std::chrono::system_clock wall_clock;
    const auto now = wall_clock::now();
    unsigned int t = wall_clock::to_time_t(now);
    return t;

    /*if (height >= future_blocktime_fork_height) {
        return t;
    }
    else {
        unsigned int t_past = get_median_time_past(height);
        return max(t, t_past + 1);
    }*/
}

unsigned int miner::get_median_time_past(uint64_t height) const
{
    block_chain_impl &block_chain = node_.chain_impl();

    int num = min<uint64_t>(height, median_time_span);
    vector<uint64_t> times;

    for (int i = 0; i < num; i++)
    {
        header header;
        if (block_chain.get_header(header, height - i - 1))
        {
            times.push_back(header.timestamp);
        }
    }

    sort(times.begin(), times.end());
    return times.empty() ? 0 : times[times.size() / 2];
}

uint64_t miner::store_block(block_ptr block)
{
    uint64_t height;
    boost::mutex mutex;
    mutex.lock();
    auto f = [&height, &mutex](const error_code &code, boost::uint64_t new_height) -> void {
        if (new_height == 0 && code.value() != 0)
            log::error(LOG_HEADER) << "store_block error: " << code.message();

        height = new_height;
        mutex.unlock();
    };
    node_.chain().store(block, f);

    boost::unique_lock<boost::mutex> lock(mutex);
    return height;
}

template <class _T>
std::string to_string(_T const &_t)
{
    std::ostringstream o;
    o << _t;
    return o.str();
}

const static BC_CONSTEXPR unsigned int num_block_per_cycle = 6;
const static BC_CONSTEXPR unsigned int num_miner_node = 2;

void miner::generate_miner_list()
{
    mine_candidate_list.clear();
    mine_address_list.clear();
    auto sh_vec = node_.chain_impl().get_registered_candidates();
    if (nullptr == sh_vec)
    {
        return;
    }

    uint64_t height = 0;
    node_.chain_impl().get_last_height(height);
    int64_t start_height = 0;
    int64_t end_height = 0;

    if (height > num_block_per_cycle * num_miner_node)
    {
        uint64_t sub_height = height % (num_block_per_cycle * num_miner_node);
        end_height = height - sub_height;
        start_height = end_height - num_block_per_cycle * num_miner_node;
    }

    for (auto &elem : *sh_vec)
    {
        auto &&rows = node_.chain_impl().get_address_history(bc::wallet::payment_address(elem.candidate.get_address()), start_height);

        chain::transaction tx_temp;
        uint64_t tx_height;

        for (auto &row : rows)
        {
            // spend unconfirmed (or no spend attempted)
            if ((row.spend.hash == null_hash) && node_.chain_impl().get_transaction(row.output.hash, tx_temp, tx_height))
            {
                BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
                const auto &output = tx_temp.outputs.at(row.output.index);
                if (output.get_script_address() != elem.candidate.get_address())
                {
                    continue;
                }
                if (output.is_vote())
                {
                    auto token_amount = output.get_token_amount();
                    uint64_t locked_amount = 0;
                    if (token_amount && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations))
                    {
                        const auto &attenuation_model_param = output.get_attenuation_model_param();
                        auto diff_height = row.output_height ? (height - row.output_height) : 0;
                        auto available_amount = attenuation_model::get_available_token_amount(
                            token_amount, diff_height, attenuation_model_param);
                        locked_amount = token_amount - available_amount;
                    }

                    if (elem.to_uid == output.get_to_uid())
                    {
                        elem.vote += token_amount;
                    }
                }
            }

            if (row.output_height >= end_height)
            {
                break;
            }
        }
    }

    //vote first and height next.
    std::sort(sh_vec->begin(), sh_vec->end(), [](const candidate_info &a, const candidate_info &b) {
        return a.vote > b.vote ? true : a.output_height > b.output_height;
    });

    for (auto &elem : *sh_vec)
    {
        mine_candidate_list.push_back(elem);

        mine_address_list.push_back(elem.candidate.get_address());

        if (mine_candidate_list.size() == num_miner_node)
        {
            break;
        }
    }
}

void miner::work(const bc::wallet::payment_address pay_address)
{
    log::info(LOG_HEADER) << "solo miner start with address: " << pay_address.encoded();

    auto sh_vec = node_.chain_impl().get_registered_candidates();
    if (nullptr == sh_vec)
    {
        log::info(LOG_HEADER) << "no candidates found ";

        return;
    }

    bool ifcandidate = false;

    for (auto &elem : *sh_vec)
    {
        if (elem.candidate.get_address() == pay_address.encoded())
        {
            ifcandidate = true;
        }
    }

    if (!ifcandidate)
    {
        log::error(LOG_HEADER) << pay_address.encoded() << " is not a candidate address ";
        return;
    }

    int64_t cycle_starttime = 0;
    while (state_ != state::exit_)
    {
        state_ = state::init_;
        auto millissecond = unix_millisecond();
        uint64_t current_block_height;

        if (node_.chain_impl().get_last_height(current_block_height))
        {
            generate_miner_list();
            int index = get_mine_index(pay_address.encoded());
            if (index == -1)
            {
                auto sleeptime = unix_millisecond() - millissecond;
                auto sleepmin = sleeptime < mine_block_produce_minsecons ? mine_block_produce_minsecons - sleeptime : 0;
                sleep_for(asio::milliseconds(sleepmin));
                continue;
            }

            /*if (current_block_height%(num_block_per_cycle*mine_address_list.size()) == 0 || millissecond-cycle_starttime >= num_block_per_cycle*mine_address_list.size()*mine_block_produce_minsecons)
            {
                cycle_starttime =  millissecond;
            }*/

            /*if (cycle_starttime == 0)
            {
                cycle_starttime = millissecond;
            }*/

            if (is_index_in_turn_with_now_height(current_block_height, index))
            {
                state_ = state::creating_block_;
                block_ptr block = create_new_block(pay_address, current_block_height);

                if (block)
                {
                    /*if (MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this, block->header.number)))
                    {*/
                    boost::uint64_t height = store_block(block);
                    if (height == 0)
                    {
                        continue;
                    }

                    log::debug(LOG_HEADER) << "solo miner create new block at heigth:" << height;

                    ++new_block_number_;
                    if ((new_block_limit_ != 0) && (new_block_number_ >= new_block_limit_))
                    {
                        thread_.reset();
                        stop();
                        break;
                    }
                    //}
                    cycle_starttime = unix_millisecond();
                }
            }
            else if (is_time_inturn_with_this_cycle(cycle_starttime))
            {
                uint16_t lost_block = get_lost_block(current_block_height, index);

                for (uint16_t i = lost_block; i > 0; i--)
                {
                    state_ = state::creating_block_;
                    block_ptr block = create_new_block(pay_address, current_block_height);

                    if (block)
                    {
                        /*if (MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this, block->header.number)))
                    {*/
                        boost::uint64_t height = store_block(block);
                        if (height == 0)
                        {
                            continue;
                        }

                        log::debug(LOG_HEADER) << "solo miner create new block at heigth:" << height;

                        ++new_block_number_;
                        if ((new_block_limit_ != 0) && (new_block_number_ >= new_block_limit_))
                        {
                            thread_.reset();
                            stop();
                            break;
                        }
                        //}
                    }
                    if (!node_.chain_impl().get_last_height(current_block_height))
                    {
                        break;
                    }
                }
            }
        }

        createblockms_ = unix_millisecond() - millissecond;
        auto sleepmin = createblockms_ < mine_block_produce_minsecons ? mine_block_produce_minsecons - createblockms_ : 0;
        //log::info(LOG_HEADER) << "solo miner create new block for " << createblockms_<<" ms";
        sleep_for(asio::milliseconds(sleepmin));
    }
}

int miner::get_mine_index(const string &pay_address) const
{
    auto it = find(mine_address_list.begin(), mine_address_list.end(), pay_address);

    if (it == mine_address_list.end())
    {
        return -1;
    }
    else
    {
        return it - mine_address_list.begin();
    }
}

bool miner::is_address_inturn(const string &pay_address) const
{
    uint64_t current_block_height;
    if (!node_.chain_impl().get_last_height(current_block_height))
    {
        return false;
    }
    const int index = get_mine_index(pay_address);
    if (index == -1)
    {
        return false;
    }
    return is_index_in_turn_with_now_height(current_block_height, index);
}

bool miner::is_address_in_turn_with_now_height(uint64_t height, const string &pay_address) const
{
    int index = get_mine_index(pay_address);
    if (index == -1)
    {
        return false;
    }
    return is_index_in_turn_with_now_height(height, index);
}

bool miner::is_index_in_turn_with_now_height(uint64_t current_block_height, const int index) const
{
    return current_block_height % (mine_address_list.size() * num_block_per_cycle) >= index * num_block_per_cycle && current_block_height % (mine_address_list.size() * num_block_per_cycle) < (index + 1) * num_block_per_cycle;
}

bool miner::is_time_inturn_with_this_cycle(int64_t cycle_starttime) const
{
    if (cycle_starttime == 0)
    {
        return false;
    }

    auto expect_time_window = mine_block_produce_minsecons * num_block_per_cycle * (mine_address_list.size() - 1);

    auto real_time_window = unix_millisecond() - cycle_starttime;

    auto time_interval = real_time_window > expect_time_window ? real_time_window - expect_time_window : 0;

    if (time_interval > mine_block_produce_minsecons)
    {
        return true;
    }

    return false;
}

uint16_t miner::get_lost_block(uint64_t height, const int index)
{
    if (mine_address_list.size() == 0)
    {
        return 0;
    }

    auto cycle_block_start = height % (num_block_per_cycle * num_miner_node);
    auto cycle_block = cycle_block_start % (num_block_per_cycle * mine_address_list.size());
    auto expect_cycle_block = index * num_block_per_cycle;
    if (expect_cycle_block == 0 && cycle_block > 0)
    {
        expect_cycle_block = num_block_per_cycle * mine_address_list.size();
    }
    return expect_cycle_block > cycle_block ? expect_cycle_block - cycle_block : 0;
}

bool miner::is_stop_miner(uint64_t block_height) const
{
    return (state_ == state::exit_) || (get_height() > block_height);
}

bool miner::start(const bc::wallet::payment_address &pay_address, uint16_t number)
{
    if (!thread_)
    {
        new_block_limit_ = number;
        thread_.reset(new boost::thread(bind(&miner::work, this, pay_address)));
    }
    pay_address_ = pay_address;
    return true;
}

bool miner::start(const std::string &public_key, uint16_t number)
{
    bc::wallet::payment_address pay_address = libbitcoin::wallet::ec_public(public_key).to_payment_address();
    if (pay_address)
    {
        return start(pay_address, number);
    }
    return false;
}

bool miner::stop()
{
    if (thread_)
    {
        state_ = state::exit_;
        thread_->join();
        thread_.reset();
    }

    state_ = state::init_;
    new_block_number_ = 0;
    new_block_limit_ = 0;
    return true;
}

uint64_t miner::get_height() const
{
    uint64_t height = 0;
    node_.chain_impl().get_last_height(height);
    return height;
}

/*bool miner::set_miner_public_key(const string& public_key)
{
    libbitcoin::wallet::ec_public ec_public_key(public_key);
    pay_address_ = ec_public_key.to_payment_address();
    if (pay_address_) {
        log::debug(LOG_HEADER) << "set_miner_public_key[" << pay_address_.encoded() << "] success";
        return true;
    }
    else {
        log::error(LOG_HEADER) << "set_miner_public_key[" << public_key << "] is not availabe!";
        return false;
    }
}*/

bool miner::set_miner_pri_key(const string &pri_key)
{
    this->pri_key = pri_key;
}

bool miner::set_miner_payment_address(const bc::wallet::payment_address &address)
{
    if (address)
    {
        log::debug(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] success";
    }
    else
    {
        log::error(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] is not availabe!";
        return false;
    }

    pay_address_ = address;
    return true;
}

const std::string miner::get_miner_address() const
{
    return pay_address_.encoded();
}

miner::block_ptr miner::get_block(bool is_force_create_block)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    if (is_force_create_block)
    {
        new_block_ = create_new_block(pay_address_);
        log::debug(LOG_HEADER) << "force create new block";
        return new_block_;
    }

    if (!new_block_)
    {
        if (pay_address_)
        {
            new_block_ = create_new_block(pay_address_);
        }
        else
        {
            log::error(LOG_HEADER) << "get_block not set pay address";
        }
    }
    else
    {
        if (get_height() >= new_block_->header.number)
        {
            new_block_ = create_new_block(pay_address_);
        }
    }

    return new_block_;
}

/*bool miner::get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary)
{
    block_ptr block = get_block();
    if (block) {
        header_hash = "0x" + to_string(HeaderAux::hashHead(new_block_->header));
        seed_hash = "0x" + to_string(HeaderAux::seedHash(new_block_->header));
        boundary = "0x" + to_string(HeaderAux::boundary(new_block_->header));
        return true;
    }
    return false;
}*/

/*bool miner::put_result(const std::string& nonce, const std::string& mix_hash,
                       const std::string& header_hash, const uint64_t &nounce_mask)
{
    bool ret = false;
    if (!get_block()) {
        return ret;
    }

    if (header_hash == "0x" + to_string(HeaderAux::hashHead(new_block_->header))) {
        auto s_nonce = "0x" + nonce;
        uint64_t n_nonce;
#ifdef MAC_OSX
        size_t sz = 0;
        n_nonce = std::stoull(s_nonce, &sz, 16);
#else
        if (sscanf(s_nonce.c_str(), "%lx", &n_nonce) != 1) {
            log::error(LOG_HEADER) << "nonce change error\n";
            return false;
        }
#endif
        // nounce_mask defination is moved to the caller by chengzhiping 2018-3-15.
        uint64_t nonce_t = n_nonce ^ nounce_mask;
        new_block_->header.nonce = (u64) nonce_t;
        new_block_->header.mixhash = (FixedHash<32>::Arith)h256(mix_hash);
        uint64_t height = store_block(new_block_);
        if (height != 0) {
            log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:"
                                   << mix_hash << " success with height:" << height;
            ret = true;
        }
        else {
            get_block(true);
            log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:" << mix_hash << " fail";
        }
    }
    else {
        log::error(LOG_HEADER) << "put_result header_hash check fail. header_hash:"
                               << header_hash << " hashHead:" << to_string(HeaderAux::hashHead(new_block_->header));
    }

    return ret;
}*/

void miner::get_state(uint64_t &height, uint32_t &miners, /*uint64_t &rate, string& difficulty,*/ bool &is_mining)
{
    //rate = MinerAux::getRate();
    block_chain_impl &block_chain = node_.chain_impl();
    header prev_header;
    block_chain.get_last_height(height);
    block_chain.get_header(prev_header, height);
    //difficulty = to_string((u256)prev_header.bits);
    is_mining = thread_ ? true : false;
    miners = mine_candidate_list.size();
}

bool miner::is_creating_block() const
{
    return state_ == state::creating_block_;
}

vector<candidate_info> &miner::get_miners()
{
    if (!thread_)
    {
        generate_miner_list();
    }

    return mine_candidate_list;
}

vector<std::string> &miner::get_miner_addresses()
{
    return mine_address_list;
}

bool miner::get_block_header(chain::header &block_header, const string &para)
{
    if (para == "pending")
    {
        block_ptr block = get_block();
        if (block)
        {
            block_header = block->header;
            return true;
        }
    }
    else if (!para.empty())
    {
        block_chain_impl &block_chain = node_.chain_impl();
        uint64_t height{0};
        if (para == "latest")
        {
            if (!block_chain.get_last_height(height))
            {
                return false;
            }
        }
        else if (para == "earliest")
        {
            height = 0;
        }
        else if (para[0] >= '0' && para[0] <= '9')
        {
            height = atol(para.c_str());
        }
        else
        {
            return false;
        }

        if (block_chain.get_header(block_header, height))
            return true;
    }

    return false;
}

} // namespace consensus
} // namespace libbitcoin
