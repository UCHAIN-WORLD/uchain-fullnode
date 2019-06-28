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
#include <UChain/blockchain/validate_block_impl.hpp>
#include <UChainService/consensus/miner.hpp>

#include <cstddef>
#include <UChain/coin.hpp>
#include <UChain/blockchain/block_info.hpp>
#include <UChain/blockchain/simple_chain.hpp>
//#include <UChainService/consensus/miner/MinerAux.h>
#include <UChain/blockchain/block_chain_impl.hpp>

namespace libbitcoin
{
namespace blockchain
{

// Value used to define median time past.
static constexpr size_t median_time_past_blocks = 11;

validate_block_impl::validate_block_impl(simple_chain &chain,
                                         size_t fork_index, const block_info::list &orphan_chain,
                                         size_t orphan_index, size_t height, const chain::block &block,
                                         bool testnet, const config::checkpoint::list &checks,
                                         stopped_callback stopped)
    : validate_block(height, block, testnet, checks, stopped),
      chain_(chain),
      height_(height),
      fork_index_(fork_index),
      orphan_index_(orphan_index),
      orphan_chain_(orphan_chain)
{
}

/*bool validate_block_impl::is_valid_proof_of_work(const chain::header& header) const
{
    chain::header parent_header;
    if (orphan_index_ != 0) {
        parent_header = orphan_chain_[orphan_index_ - 1]->actual()->header;
    }
    else {
        static_cast<block_chain_impl&>(chain_).get_header(parent_header, header.number - 1);
    }
    return MinerAux::verifySeal(const_cast<chain::header&>(header), parent_header);
}*/

/*u256 validate_block_impl::previous_block_bits() const
{
    // Read block header (top - 1) and return bits
    return fetch_block(height_ - 1).bits;
}*/

validate_block::versions validate_block_impl::preceding_block_versions(
    size_t maximum) const
{
    // 1000 previous versions maximum sample.
    // 950 previous versions minimum required for enforcement.
    // 750 previous versions minimum required for activation.
    const auto size = std::min(maximum, height_);

    // Read block (top - 1) through (top - 1000) and return version vector.
    versions result;
    for (size_t index = 0; index < size; ++index)
    {
        const auto version = fetch_block(height_ - index - 1).version;

        // Some blocks have high versions, see block #390777.
        static const auto maximum = static_cast<uint32_t>(max_uint8);
        const auto normal = std::min(version, maximum);
        result.push_back(static_cast<uint8_t>(normal));
    }

    return result;
}

uint64_t validate_block_impl::actual_time_span(size_t interval) const
{
    BITCOIN_ASSERT(height_ > 0 && height_ >= interval);

    // height - interval and height - 1, return time difference
    return fetch_block(height_ - 1).timestamp - fetch_block(height_ - interval).timestamp;
}

uint64_t validate_block_impl::median_time_past() const
{
    // Read last 11 (or height if height < 11) block times into array.
    const auto count = std::min(height_, median_time_past_blocks);

    std::vector<uint64_t> times;
    for (size_t i = 0; i < count; ++i)
        times.push_back(fetch_block(height_ - i - 1).timestamp);

    // Sort and select middle (median) value from the array.
    std::sort(times.begin(), times.end());
    return times.empty() ? 0 : times[times.size() / 2];
}

chain::header validate_block_impl::fetch_block(size_t fetch_height) const
{
    if (fetch_height > fork_index_)
    {
        const auto fetch_index = fetch_height - fork_index_ - 1;
        BITCOIN_ASSERT(fetch_index <= orphan_index_);
        BITCOIN_ASSERT(orphan_index_ < orphan_chain_.size());
        return orphan_chain_[fetch_index]->actual()->header;
    }

    chain::header out;
    DEBUG_ONLY(const auto result =)
    chain_.get_header(out, fetch_height);
    BITCOIN_ASSERT(result);
    return out;
}

bool tx_after_fork(size_t tx_height, size_t fork_index)
{
    return tx_height > fork_index;
}

bool validate_block_impl::transaction_exists(const hash_digest &tx_hash) const
{
    uint64_t out_height;
    chain::transaction unused;
    const auto result = chain_.get_transaction(unused, out_height, tx_hash);
    if (!result)
        return false;

    BITCOIN_ASSERT(out_height <= max_size_t);
    const auto tx_height = static_cast<size_t>(out_height);
    return tx_height <= fork_index_;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point &outpoint) const
{
    hash_digest out_hash;
    const auto result = chain_.get_outpoint_transaction(out_hash, outpoint);
    if (!result)
        return false;

    // Lookup block height. Is the spend after the fork point?
    return transaction_exists(out_hash);
}

bool validate_block_impl::fetch_transaction(chain::transaction &tx,
                                            size_t &tx_height, const hash_digest &tx_hash) const
{
    uint64_t out_height;
    const auto result = chain_.get_transaction(tx, out_height, tx_hash);

    BITCOIN_ASSERT(out_height <= max_size_t);
    tx_height = static_cast<size_t>(out_height);

    if (!result || tx_after_fork(tx_height, fork_index_))
    {
        return fetch_orphan_transaction(tx, tx_height, tx_hash);
    }

    return true;
}

bool validate_block_impl::fetch_orphan_transaction(chain::transaction &tx,
                                                   size_t &tx_height, const hash_digest &tx_hash) const
{
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            if (orphan_tx.hash() == tx_hash)
            {
                // TRANSACTION COPY
                tx = orphan_tx;
                tx_height = fork_index_ + orphan + 1;
                return true;
            }
        }
    }

    return false;
}

std::string validate_block_impl::get_uid_from_address_consider_orphan_chain(
    const std::string &address, const std::string &uid_symbol) const
{
    BITCOIN_ASSERT(!address.empty());

    if (address.empty())
    {
        log::debug("blockchain") << "get_uid_from_address_consider_orphan_chain: address is empty";
        return "";
    }

    auto orphan = orphan_index_;
    while (orphan > 0)
    {
        const auto &orphan_block = orphan_chain_[--orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            // iter outputs
            for (const auto &output : orphan_tx.outputs)
            {
                if (output.is_uid_register() || output.is_uid_transfer())
                {
                    if (address == output.get_uid_address())
                    {
                        return output.get_uid_symbol();
                    }
                }
            }

            // iter inputs
            for (const auto &input : orphan_tx.inputs)
            {
                size_t previous_height;
                transaction previous_tx;
                const auto &previous_output = input.previous_output;

                // This searches the blockchain and then the orphan pool up to and
                // including the current (orphan) block and excluding blocks above fork.
                if (!fetch_transaction(previous_tx, previous_height, previous_output.hash))
                {
                    log::warning(LOG_BLOCKCHAIN)
                        << "Failure fetching input transaction ["
                        << encode_hash(previous_output.hash) << "]";
                    return "";
                }

                const auto &previous_tx_out = previous_tx.outputs[previous_output.index];

                if (previous_tx_out.is_uid_register() || previous_tx_out.is_uid_transfer())
                {
                    if (address == previous_tx_out.get_uid_address())
                    {
                        return "";
                    }
                }
            }
        }
    }

    return uid_symbol;
}

bool validate_block_impl::is_uid_match_address_in_orphan_chain(const std::string &uid, const std::string &address) const
{
    BITCOIN_ASSERT(!uid.empty());
    BITCOIN_ASSERT(!address.empty());

    if (address.empty())
    {
        log::debug("blockchain") << "check uid match address in orphan chain: address is null for uid: " << uid;
        return false;
    }

    auto orphan = orphan_index_;
    while (orphan > 0)
    {
        const auto &orphan_block = orphan_chain_[--orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            for (auto &output : orphan_tx.outputs)
            {
                if (output.is_uid_register() || output.is_uid_transfer())
                {
                    if (uid == output.get_uid_symbol())
                    {
                        return address == output.get_uid_address();
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_uid_in_orphan_chain(const std::string &uid) const
{
    BITCOIN_ASSERT(!uid.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            for (auto &output : orphan_tx.outputs)
            {
                if (output.is_uid_register())
                {
                    if (uid == output.get_uid_symbol())
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_token_in_orphan_chain(const std::string &symbol) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            for (const auto &output : orphan_tx.outputs)
            {
                if (output.is_token_issue())
                {
                    if (symbol == output.get_token_symbol())
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_token_cert_in_orphan_chain(const std::string &symbol, token_cert_type cert_type) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            for (const auto &output : orphan_tx.outputs)
            {
                if (symbol == output.get_token_cert_symbol() &&
                    cert_type == output.get_token_cert_type())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_candidate_in_orphan_chain(const std::string &symbol) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        for (const auto &orphan_tx : orphan_block->transactions)
        {
            for (const auto &output : orphan_tx.outputs)
            {
                if (output.is_candidate_register())
                {
                    if (symbol == output.get_candidate_symbol())
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point &previous_output,
    size_t index_in_parent, size_t input_index) const
{
    // Search for double spends. This must be done in both chain AND orphan.
    // Searching chain when this tx is an orphan is redundant but it does not
    // happen enough to care.
    if (is_output_spent(previous_output))
        return true;

    if (orphan_is_spent(previous_output, index_in_parent, input_index))
        return true;

    return false;
}

bool validate_block_impl::orphan_is_spent(
    const chain::output_point &previous_output,
    size_t skip_tx, size_t skip_input) const
{
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan)
    {
        const auto &orphan_block = orphan_chain_[orphan]->actual();
        const auto &transactions = orphan_block->transactions;

        BITCOIN_ASSERT(!transactions.empty());
        BITCOIN_ASSERT(transactions.front().is_coinbase());

        for (size_t tx_index = 0; tx_index < transactions.size(); ++tx_index)
        {
            // TODO: too deep, move this section to subfunction.
            const auto &orphan_tx = transactions[tx_index];

            for (size_t input_index = 0; input_index < orphan_tx.inputs.size(); ++input_index)
            {
                const auto &orphan_input = orphan_tx.inputs[input_index];

                if (orphan == orphan_index_ && tx_index == skip_tx && input_index == skip_input)
                    continue;

                if (orphan_input.previous_output == previous_output)
                    return true;
            }
        }
    }

    return false;
}

bool validate_block_impl::check_get_coinage_reward_transaction(const chain::transaction &coinage_reward_coinbase, const chain::output &output, bool is_candidate) const
{
    uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
    uint64_t coinbase_lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(coinage_reward_coinbase.outputs[0].script.operations);
    bc::wallet::payment_address addr1 = bc::wallet::payment_address::extract(coinage_reward_coinbase.outputs[0].script);
    bc::wallet::payment_address addr2 = bc::wallet::payment_address::extract(output.script);
    uint64_t coinage_reward_value = libbitcoin::consensus::miner::calculate_lockblock_reward(lock_height, output.value);

    if (is_candidate && output.value != bc::min_lock_to_issue_candidate)
    {
        return false;
    }
    else
    {
        return (is_candidate || addr1 == addr2) && lock_height == coinbase_lock_height && coinage_reward_value == coinage_reward_coinbase.outputs[0].value;
    }
}

} // namespace blockchain
} // namespace libbitcoin
