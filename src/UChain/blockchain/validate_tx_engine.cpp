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
#include <UChain/blockchain/validate_tx_engine.hpp>
#include <UChain/coin/chain/script/operation.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <memory>
#include <UChain/coin.hpp>
#include <UChain/blockchain/tx_pool.hpp>
#include <UChain/blockchain/validate_block.hpp>
#include <UChainService/consensus/miner.hpp>

#ifdef WITH_CONSENSUS
#include <UChainService/consensus.hpp>
#endif

namespace libbitcoin
{
namespace blockchain
{

static BC_CONSTEXPR unsigned int min_tx_fee = 10000;

using namespace chain;
using namespace std::placeholders;

// Max transaction size is set to max block size (1,000,000).
static constexpr uint32_t max_transaction_size = 1000000;

validate_tx_engine::validate_tx_engine(block_chain &chain,
                                           const chain::transaction &tx, const validate_block &validate_block)
    : blockchain_(static_cast<blockchain::block_chain_impl &>(chain)),
      tx_(std::make_shared<message::tx_message>(tx)),
      pool_(nullptr),
      dispatch_(nullptr),
      validate_block_(&validate_block),
      tx_hash_(tx.hash())
{
}

validate_tx_engine::validate_tx_engine(block_chain &chain,
                                           const chain::transaction &tx, const tx_pool &pool, dispatcher &dispatch)
    : blockchain_(static_cast<blockchain::block_chain_impl &>(chain)),
      tx_(std::make_shared<message::tx_message>(tx)),
      pool_(&pool),
      dispatch_(&dispatch),
      validate_block_(nullptr),
      tx_hash_(tx.hash())
{
}

void validate_tx_engine::start(validate_handler handler)
{
    BITCOIN_ASSERT(tx_ && pool_ && dispatch_);

    handle_validate_ = handler;
    const auto ec = basic_checks();

    if (ec)
    {
        if (ec == error::input_not_found)
        {
            handle_validate_(ec, tx_, {current_input_});
            return;
        }

        handle_validate_(ec, tx_, {});
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    // TODO: change to fetch_unspent_transaction, spent dups ok (BIP30).
    ///////////////////////////////////////////////////////////////////////////
    // Check for duplicates in the blockchain.
    blockchain_.fetch_transaction(tx_hash_,
                                  dispatch_->unordered_delegate(
                                      &validate_tx_engine::handle_duplicate_check,
                                      shared_from_this(), _1));
}

code validate_tx_engine::basic_checks() const
{
    const auto ec = check_transaction();

    if (ec)
        return ec;

    // This should probably preceed check_transaction.
    if (tx_->is_strict_coinbase())
        return error::coinbase_transaction;

    // Ummm...
    //if ((int64)nLockTime > INT_MAX)

    if (!is_standard())
        return error::is_not_standard;

    if (pool_->is_in_pool(tx_hash_))
        return error::duplicate;

    // Check for blockchain duplicates in start (after this returns).
    return error::success;
}

bool validate_tx_engine::is_standard() const
{
    return true;
}

void validate_tx_engine::handle_duplicate_check(
    const code &ec)
{
    if (ec != (code)error::not_found)
    {
        ///////////////////////////////////////////////////////////////////////
        // BUGBUG: overly restrictive, spent dups ok (BIP30).
        ///////////////////////////////////////////////////////////////////////
        handle_validate_(error::duplicate, tx_, {});
        return;
    }

    // TODO: we may want to allow spent-in-pool (RBF).
    if (pool_->is_spent_in_pool(tx_))
    {
        handle_validate_(error::double_spend, tx_, {});
        return;
    }

    // Check inputs, we already know it is not a coinbase tx.
    blockchain_.fetch_last_height(
        dispatch_->unordered_delegate(&validate_tx_engine::set_last_height,
                                      shared_from_this(), _1, _2));
}

void validate_tx_engine::reset(size_t last_height)
{
    // Used for checking coinbase maturity
    last_block_height_ = last_height;
    current_input_ = 0;
    value_in_ = 0;
    token_amount_in_ = 0;
    token_certs_in_.clear();
    old_symbol_in_ = "";
    old_cert_symbol_in_ = "";
}

void validate_tx_engine::set_last_height(const code &ec,
                                           size_t last_height)
{
    if (ec)
    {
        handle_validate_(ec, tx_, {});
        return;
    }

    reset(last_height);

    // Begin looping through the inputs, fetching the previous tx.
    if (!tx_->inputs.empty())
        next_previous_transaction();
}

void validate_tx_engine::next_previous_transaction()
{
    BITCOIN_ASSERT(current_input_ < tx_->inputs.size());

    // First we fetch the parent block height for a transaction.
    // Needed for checking the coinbase maturity.
    blockchain_.fetch_transaction_index(
        tx_->inputs[current_input_].previous_output.hash,
        dispatch_->unordered_delegate(
            &validate_tx_engine::previous_tx_index,
            shared_from_this(), _1, _2));
}

void validate_tx_engine::previous_tx_index(const code &ec,
                                             size_t parent_height)
{
    if (ec)
    {
        search_pool_previous_tx();
        return;
    }

    BITCOIN_ASSERT(current_input_ < tx_->inputs.size());
    const auto &prev_tx_hash = tx_->inputs[current_input_].previous_output.hash;

    // Now fetch actual transaction body
    blockchain_.fetch_transaction(prev_tx_hash,
                                  dispatch_->unordered_delegate(&validate_tx_engine::handle_previous_tx,
                                                                shared_from_this(), _1, _2, parent_height));
}

bool validate_tx_engine::get_previous_tx(chain::transaction &prev_tx,
                                           uint64_t &prev_height, const chain::input &input) const
{
    prev_height = 0;
    if (pool_)
    {
        if (blockchain_.get_transaction(prev_tx, prev_height, input.previous_output.hash))
        {
            return true; // find in block chain
        }
        if (pool_->find(prev_tx, input.previous_output.hash))
        {
            return true; // find in memory pool
        }
    }
    else
    {
        size_t temp_height = 0;
        if (validate_block_ &&
            validate_block_->get_transaction(input.previous_output.hash, prev_tx, temp_height))
        {
            prev_height = temp_height;
            return true; // find in block chain or orphan pool
        }
    }
    return false; // failed
}

void validate_tx_engine::search_pool_previous_tx()
{
    transaction previous_tx;
    const auto &current_input = tx_->inputs[current_input_];

    if (!pool_->find(previous_tx, current_input.previous_output.hash))
    {
        log::debug(LOG_BLOCKCHAIN) << "search_pool_previous_tx failed: prev hash"
                                   << encode_hash(current_input.previous_output.hash);
        const auto list = point::indexes{current_input_};
        handle_validate_(error::input_not_found, tx_, list);
        return;
    }

    // parent_height ignored here as mempool transactions cannot be coinbase.
    BITCOIN_ASSERT(!previous_tx.is_coinbase());
    static constexpr size_t parent_height = 0;
    handle_previous_tx(error::success, previous_tx, parent_height);
    unconfirmed_.push_back(current_input_);
}

void validate_tx_engine::handle_previous_tx(const code &ec,
                                              const transaction &previous_tx, size_t parent_height)
{
    if (ec)
    {
        log::debug(LOG_BLOCKCHAIN) << "handle_previous_tx failed: error: "
                                   << std::to_string(ec.value()) << ", prev hash: "
                                   << encode_hash(previous_tx.hash());
        const auto list = point::indexes{current_input_};
        handle_validate_(error::input_not_found, tx_, list);
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    // HACK: this assumes that the mempool is operating at min block version 4.
    ///////////////////////////////////////////////////////////////////////////

    // Should check if inputs are standard here...
    if (!connect_input(previous_tx, parent_height))
    {
        log::debug(LOG_BLOCKCHAIN) << "connect_input of transaction failed. prev tx hash:"
                                   << encode_hash(previous_tx.hash());
        const auto list = point::indexes{current_input_};
        handle_validate_(error::validate_inputs_failed, tx_, list);
        return;
    }

    // Search for double spends...
    blockchain_.fetch_spend(tx_->inputs[current_input_].previous_output,
                            dispatch_->unordered_delegate(&validate_tx_engine::check_double_spend,
                                                          shared_from_this(), _1, _2));
}

void validate_tx_engine::check_double_spend(const code &ec,
                                              const chain::input_point &)
{
    if (ec != (code)error::unspent_output)
    {
        handle_validate_(error::double_spend, tx_, {});
        return;
    }

    // End of connect_input checks.
    ++current_input_;
    if (current_input_ < tx_->inputs.size())
    {
        next_previous_transaction();
        return;
    }

    // current_input_ will be invalid on last pass.
    check_fees();
}

void validate_tx_engine::check_fees() const
{
    code ec = check_tx_connect_input();
    if (ec != error::success)
    {
        handle_validate_(ec, tx_, {});
        return;
    }
    if (tx_->has_token_vote())
    {
        code eo = check_tx_connect_output();
        if (eo != error::success)
        {
            handle_validate_(eo, tx_, {});
            return;
        }
    }

    // Who cares?
    // Fuck the police
    // Every tx equal!
    handle_validate_(error::success, tx_, unconfirmed_);
}

code validate_tx_engine::check_tx_connect_input() const
{
    uint64_t fee = 0;

    if (!tally_fees(blockchain_, *tx_, value_in_, fee))
    {
        return error::fees_out_of_range;
    }

    if (tx_->has_token_transfer())
    {
        if (!check_token_amount(*tx_))
        {
            return error::token_amount_not_equal;
        }
        if (!check_token_symbol(*tx_))
        {
            return error::token_symbol_not_match;
        }
    }

    if (tx_->has_token_cert())
    {
        if (!check_token_certs(*tx_))
        {
            log::debug(LOG_BLOCKCHAIN) << "failed to check token cert." << tx_->to_string(1);
            return error::token_cert_error;
        }
    }

    if (tx_->has_candidate_transfer())
    {
        if (!check_candidate(*tx_))
        {
            log::debug(LOG_BLOCKCHAIN) << "failed to check candidate token." << tx_->to_string(1);
            return error::candidate_error;
        }
    }

    if (tx_->has_uid_transfer())
    {
        if (!check_uid_symbol_match(*tx_))
        {
            return error::uid_symbol_not_match;
        }
    }

    return error::success;
}

code validate_tx_engine::check_tx_connect_output() const
{
    uint64_t value = 0, quatity = 0, lock_height = 0;

    for (auto &ele : tx_->outputs)
    {
        if (chain::operation::is_pay_key_hash_with_lock_height_pattern(ele.script.operations))
        {
            lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(ele.script.operations);
            if (lock_height == VOTE_LOCKED_TIME)
                value += ele.value;
        }
        if (ele.is_vote())
        {
            auto &&token_transfer = ele.get_token_transfer();
            quatity += token_transfer.get_quantity();
        }
    }

    if (quatity * TIMES_QUANTITY_TO_VALUE != value)
    { // TODO: for debug
        return error::invalid_quantity_or_value;
    }
    return error::success;
}

static bool check_same(std::string &dest, const std::string &src)
{
    if (dest.empty())
    {
        dest = src;
    }
    else if (dest != src)
    {
        log::debug(LOG_BLOCKCHAIN) << "check_same: " << dest << " != " << src;
        return false;
    }
    return true;
}

code validate_tx_engine::check_secondaryissue_transaction() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &blockchain = blockchain_;

    bool is_token_secondaryissue{false};
    for (auto &output : tx.outputs)
    {
        if (output.is_token_secondaryissue())
        {
            is_token_secondaryissue = true;
            break;
        }
    }
    if (!is_token_secondaryissue)
    {
        return error::success;
    }

    is_token_secondaryissue = false;
    std::string token_symbol;
    std::string token_address;
    std::string token_cert_owner;
    uint8_t secondaryissue_threshold{0};
    uint64_t secondaryissue_token_amount{0};
    uint64_t token_transfer_volume{0};
    int num_token_secondaryissue{0};
    int num_token_transfer{0};
    int num_token_cert{0};
    std::vector<token_cert_type> certs_out;
    for (auto &output : tx.outputs)
    {
        if (output.is_token_secondaryissue())
        {
            ++num_token_secondaryissue;
            if (num_token_secondaryissue > 1)
            {
                log::debug(LOG_BLOCKCHAIN) << "secondaryissue: num of secondaryissue output > 1, " << token_symbol;
                return error::token_secondaryissue_error;
            }

            auto &&token_detail = output.get_token_detail();
            if (!token_detail.is_token_secondaryissue() || !token_detail.is_secondaryissue_threshold_value_ok())
            {
                log::debug(LOG_BLOCKCHAIN) << "secondaryissue: threshold value invalid, " << token_symbol;
                return error::token_secondaryissue_threshold_invalid;
            }
            if (!check_same(token_symbol, token_detail.get_symbol()))
            {
                return error::token_secondaryissue_error;
            }
            if (!check_same(token_address, token_detail.get_address()))
            {
                return error::token_secondaryissue_error;
            }
            if (operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations))
            {
                const auto &model_param = output.get_attenuation_model_param();
                if (!attenuation_model::check_model_param(
                        model_param, token_detail.get_maximum_supply()))
                {
                    log::debug(LOG_BLOCKCHAIN) << "secondaryissue: model param invalid, "
                                               << token_symbol << " " << model_param;
                    return error::attenuation_model_param_error;
                }
            }
            secondaryissue_threshold = token_detail.get_secondaryissue_threshold();
            secondaryissue_token_amount = token_detail.get_maximum_supply();
        }
        else if (output.is_token_transfer())
        {
            ++num_token_transfer;
            auto &&token_transfer = output.get_token_transfer();
            if (!check_same(token_symbol, token_transfer.get_symbol()))
            {
                return error::token_secondaryissue_error;
            }
            if (!check_same(token_address, output.get_script_address()))
            {
                return error::token_secondaryissue_error;
            }
            token_transfer_volume += token_transfer.get_quantity();
        }
        else if (output.is_token_cert())
        {
            ++num_token_cert;
            if (num_token_cert > 1)
            {
                log::debug(LOG_BLOCKCHAIN) << "secondaryissue: cert numbers > 1, " << token_symbol;
                return error::token_secondaryissue_error;
            }
            auto &&token_cert = output.get_token_cert();
            auto cur_cert_type = token_cert.get_type();
            if (cur_cert_type == token_cert_ns::issue)
            {
                if (!check_same(token_symbol, token_cert.get_symbol()))
                {
                    return error::token_secondaryissue_error;
                }
                if (!check_same(token_cert_owner, token_cert.get_owner()))
                {
                    return error::token_secondaryissue_error;
                }
                certs_out.push_back(cur_cert_type);
            }
            else
            {
                log::debug(LOG_BLOCKCHAIN) << "secondaryissue: invalid output of cert "
                                           << token_cert.to_string();
                return error::token_secondaryissue_error;
            }
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            log::debug(LOG_BLOCKCHAIN) << "secondaryissue: illega output, "
                                       << token_symbol << " : " << output.to_string(1);
            return error::token_secondaryissue_error;
        }
    }

    if (tx.version >= transaction_version::check_uid_feature && !token_cert::test_certs(certs_out, token_cert_ns::issue))
    {
        log::debug(LOG_BLOCKCHAIN) << "secondaryissue: no issue token cert, " << token_symbol;
        return error::token_cert_error;
    }

    auto total_volume = blockchain.get_token_volume(token_symbol);
    if (total_volume > max_uint64 - secondaryissue_token_amount)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "secondaryissue: total token volume cannot exceed maximum value, "
            << token_symbol;
        return error::token_secondaryissue_error;
    }

    if (!token_detail::is_secondaryissue_owns_enough(token_transfer_volume, total_volume, secondaryissue_threshold))
    {
        log::debug(LOG_BLOCKCHAIN) << "secondaryissue: no enough token volume, " << token_symbol;
        return error::token_secondaryissue_share_not_enough;
    }

    // check inputs token address
    for (const auto &input : tx.inputs)
    {
        chain::transaction prev_tx;
        uint64_t prev_height{0};
        if (!get_previous_tx(prev_tx, prev_height, input))
        {
            log::debug(LOG_BLOCKCHAIN) << "secondaryissue: input not found: "
                                       << encode_hash(input.previous_output.hash);
            return error::input_not_found;
        }
        auto prev_output = prev_tx.outputs.at(input.previous_output.index);
        if (prev_output.is_token() || prev_output.is_token_cert())
        {
            auto &&token_address_in = prev_output.get_script_address();
            if (prev_output.is_token_cert())
            {
                auto &&prev_token_cert = prev_output.get_token_cert();
                if (prev_token_cert.get_symbol() != token_symbol || prev_token_cert.get_type() != token_cert_ns::issue)
                {
                    log::debug(LOG_BLOCKCHAIN) << "secondaryissue: invalid cert input, " << token_symbol;
                    return error::validate_inputs_failed;
                }
            }
            else if (token_address != token_address_in)
            {
                log::debug(LOG_BLOCKCHAIN) << "secondaryissue: invalid token input, " << token_symbol;
                return error::validate_inputs_failed;
            }
        }
    }

    return error::success;
}

code validate_tx_engine::check_token_issue_transaction() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;

    bool is_token_issue{false};
    for (auto &output : tx.outputs)
    {
        if (output.is_token_issue())
        {
            is_token_issue = true;
            break;
        }
    }
    if (!is_token_issue)
    {
        return error::success;
    }

    is_token_issue = false;
    int num_cert_issue{0};
    int num_cert_domain_or_naming{0};
    std::vector<token_cert_type> cert_mask;
    std::vector<token_cert_type> cert_type;
    std::string token_symbol;
    std::string token_address;
    std::string cert_owner;
    for (auto &output : tx.outputs)
    {
        if (output.is_token_issue())
        {
            if (is_token_issue)
            {
                // can not issue multiple tokens at the same transaction
                return error::token_issue_error;
            }
            is_token_issue = true;
            token_detail &&detail = output.get_token_detail();
            if (!detail.is_secondaryissue_threshold_value_ok())
            {
                return error::token_secondaryissue_threshold_invalid;
            }
            if (!check_same(token_symbol, detail.get_symbol()))
            {
                return error::token_issue_error;
            }
            if (!check_same(token_address, detail.get_address()))
            {
                return error::token_issue_error;
            }
            if (check_token_exist(token_symbol))
            {
                return error::token_exist;
            }
            if (operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations))
            {
                const auto &model_param = output.get_attenuation_model_param();
                if (!attenuation_model::check_model_param(
                        model_param, detail.get_maximum_supply()))
                {
                    log::debug(LOG_BLOCKCHAIN) << "issue: model param invalid, "
                                               << token_symbol << " " << model_param;
                    return error::attenuation_model_param_error;
                }
            }
            cert_mask = detail.get_token_cert_mask();
        }
        else if (output.is_token_cert())
        {
            token_cert &&cert_info = output.get_token_cert();

            // check cert
            token_cert_type cur_cert_type = cert_info.get_type();
            if (cur_cert_type == token_cert_ns::issue)
            {
                ++num_cert_issue;
                if (num_cert_issue > 1)
                {
                    return error::token_issue_error;
                }

                if (!check_same(token_symbol, cert_info.get_symbol()))
                {
                    return error::token_issue_error;
                }

                if (!check_same(token_address, output.get_script_address()))
                {
                    return error::token_issue_error;
                }
            }
            else if (cur_cert_type == token_cert_ns::domain)
            {
                ++num_cert_domain_or_naming;
                if (num_cert_domain_or_naming > 1)
                {
                    return error::token_issue_error;
                }

                if (!token_symbol.empty())
                {
                    auto &&domain = token_cert::get_domain(token_symbol);
                    if (domain != cert_info.get_symbol())
                    {
                        return error::token_issue_error;
                    }
                }

                if (!check_same(cert_owner, cert_info.get_owner()))
                {
                    return error::token_issue_error;
                }
            }
            else if (cur_cert_type == token_cert_ns::naming)
            {
                ++num_cert_domain_or_naming;
                if (num_cert_domain_or_naming > 1)
                {
                    return error::token_issue_error;
                }

                if (!check_same(token_symbol, cert_info.get_symbol()))
                {
                    return error::token_issue_error;
                }

                if (!check_same(cert_owner, cert_info.get_owner()))
                {
                    return error::token_issue_error;
                }
            }
            else
            {
                log::debug(LOG_BLOCKCHAIN) << "issue: invalid output of cert "
                                           << cert_info.to_string();
                return error::token_issue_error;
            }

            cert_type.push_back(cur_cert_type);
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            log::debug(LOG_BLOCKCHAIN) << "issue: illega output, "
                                       << token_symbol << " : " << output.to_string(1);
            return error::token_issue_error;
        }
    }

    // check cert for transactions after check_uid_feature version.
    if (tx.version >= transaction_version::check_uid_feature)
    {
        if (!token_cert::test_certs(cert_type, cert_mask))
        {
            log::debug(LOG_BLOCKCHAIN) << "issue token: "
                                       << "not enough cert.";
            return error::token_issue_error;
        }

        auto &&domain = token_cert::get_domain(token_symbol);
        if (token_cert::is_valid_domain(domain))
        {
            if (cert_owner.empty())
            {
                log::debug(LOG_BLOCKCHAIN) << "issue token: owner of cert "
                                           << token_symbol << " is empty!";
                return error::token_cert_error;
            }

            if (num_cert_domain_or_naming < 1)
            {
                // no valid domain or naming cert
                log::debug(LOG_BLOCKCHAIN) << "issue token: not cert provided!";
                return error::token_cert_not_provided;
            }
        }
    }

    return error::success;
}

code validate_tx_engine::check_token_cert_transaction() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;

    bool is_cert{false};
    for (auto &output : tx.outputs)
    {
        if (output.is_token_cert_issue() || output.is_token_cert_transfer())
        {
            is_cert = true;
            break;
        }
    }

    if (!is_cert)
    {
        return error::success;
    }

    int num_cert_issue{0};
    int num_cert_domain{0};
    int num_cert_transfer{0};
    token_cert_type issue_cert_type{token_cert_ns::none};
    std::vector<token_cert_type> cert_type;
    std::string cert_symbol;
    std::string domain_symbol;
    std::string cert_owner;
    for (auto &output : tx.outputs)
    {
        if (output.is_token_cert_issue())
        {
            ++num_cert_issue;
            if (num_cert_issue > 1)
            {
                // can not issue multiple token cert at the same transaction
                return error::token_cert_issue_error;
            }

            token_cert &&cert_info = output.get_token_cert();
            token_cert_type cur_cert_type = cert_info.get_type();

            if (!check_same(cert_symbol, cert_info.get_symbol()))
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: "
                                           << cert_info.get_symbol() << " does not match.";
                return error::token_cert_issue_error;
            }

            // check cert not exists
            if (check_token_cert_exist(cert_symbol, cur_cert_type))
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: "
                                           << cert_info.get_symbol() << " already exists.";
                return error::token_cert_exist;
            }

            issue_cert_type = cur_cert_type;
        }
        else if (output.is_token_cert_transfer())
        {
            ++num_cert_transfer;
            if (num_cert_transfer > 1)
            {
                // can not transfer multiple token cert at the same transaction
                return error::token_cert_error;
            }

            token_cert &&cert_info = output.get_token_cert();
            if (!check_same(cert_symbol, cert_info.get_symbol()))
            {
                log::debug(LOG_BLOCKCHAIN) << "transfer cert: "
                                           << cert_info.get_symbol() << " does not match.";
                return error::token_cert_error;
            }
        }
        else if (output.is_token_cert())
        {
            token_cert &&cert_info = output.get_token_cert();

            // check cert
            token_cert_type cur_cert_type = cert_info.get_type();
            if (cur_cert_type == token_cert_ns::domain)
            {
                if (issue_cert_type != token_cert_ns::naming)
                {
                    log::debug(LOG_BLOCKCHAIN) << "issue cert: redundant output of domain cert.";
                    return error::token_cert_issue_error;
                }

                ++num_cert_domain;
                if (num_cert_domain > 1)
                {
                    return error::token_cert_issue_error;
                }

                domain_symbol = cert_info.get_symbol();

                // check owner
                cert_owner = cert_info.get_owner();
                auto uiddetail = chain.get_registered_uid(cert_owner);
                auto address = cert_info.get_address();
                if (!uiddetail)
                {
                    log::debug(LOG_BLOCKCHAIN) << "issue cert: cert owner is not issued. "
                                               << cert_info.to_string();
                    return error::token_cert_issue_error;
                }
                if (address != uiddetail->get_address())
                {
                    log::debug(LOG_BLOCKCHAIN) << "issue cert: cert address dismatch cert owner. "
                                               << cert_info.to_string();
                    return error::token_cert_issue_error;
                }
            }
            else
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: invalid output of cert "
                                           << cert_info.to_string();
                return error::token_cert_issue_error;
            }

            cert_type.push_back(cur_cert_type);
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            log::debug(LOG_BLOCKCHAIN) << "cert: illegal output asset type:"
                                       << output.attach_data.get_type()
                                       << ", tx: " << output.to_string(1);
            return error::token_cert_issue_error;
        }
    }

    if ((num_cert_issue == 0 && num_cert_transfer == 0) || (num_cert_issue > 0 && num_cert_transfer > 0) || (num_cert_transfer > 0 && num_cert_domain > 0))
    {
        log::debug(LOG_BLOCKCHAIN) << "cert: illegal output.";
        return error::token_cert_error;
    }

    if (num_cert_issue == 1)
    {
        if (issue_cert_type == token_cert_ns::none)
        {
            return error::token_cert_issue_error;
        }

        if (issue_cert_type == token_cert_ns::naming)
        {
            if (!token_cert::test_certs(cert_type, token_cert_ns::domain) || cert_owner.empty())
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: "
                                           << "no domain cert provided to issue naming cert.";
                return error::token_cert_issue_error;
            }

            auto &&domain = token_cert::get_domain(cert_symbol);
            if (domain != domain_symbol)
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: "
                                           << "invalid domain cert provided to issue naming cert.";
                return error::token_cert_issue_error;
            }

            // check token not exist.
            if (check_token_exist(cert_symbol))
            {
                log::debug(LOG_BLOCKCHAIN) << "issue cert: "
                                           << "token symbol '" + cert_symbol + "' already exists in blockchain!";
                return error::token_exist;
            }
        }
    }

    return error::success;
}

code validate_tx_engine::check_candidate_transaction() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;

    bool is_candidate{false};
    for (auto &output : tx.outputs)
    {
        if (output.is_candidate())
        {
            is_candidate = true;
            break;
        }
    }

    if (!is_candidate)
    {
        return error::success;
    }

    std::string token_symbol;
    std::string token_address;
    size_t num_candidate_transfer = 0;
    size_t num_candidate_register = 0;
    for (auto &output : tx.outputs)
    {
        if (output.is_candidate_register())
        {
            ++num_candidate_register;

            auto &&token_info = output.get_candidate();
            token_symbol = token_info.get_symbol();

            if (!check_same(token_address, token_info.get_address()))
            {
                log::debug(LOG_BLOCKCHAIN) << "register candidate: "
                                           << " address is not same. "
                                           << token_address << " != " << token_info.get_address();
                return error::candidate_register_error;
            }

            // check token not exists
            if (check_candidate_exist(token_symbol))
            {
                log::debug(LOG_BLOCKCHAIN) << "register candidate: "
                                           << token_symbol << " already exists.";
                return error::candidate_exist;
            }
        }
        else if (output.is_candidate_transfer())
        {
            if (++num_candidate_transfer > 1)
            {
                log::debug(LOG_BLOCKCHAIN) << "transfer candidate: more than on candidate output." << output.to_string(1);
                return error::candidate_error;
            }

            auto &&token_info = output.get_candidate();
            token_symbol = token_info.get_symbol();
        }
        else if (output.is_ucn())
        {
            if (!check_same(token_address, output.get_script_address()))
            {
                log::debug(LOG_BLOCKCHAIN) << "candidate: "
                                           << " address is not same. "
                                           << token_address << " != " << output.get_script_address();
                return error::candidate_register_error;
            }
        }
        else if (!output.is_message())
        {
            log::debug(LOG_BLOCKCHAIN) << "candidate: illegal output, "
                                       << token_symbol << " : " << output.to_string(1);
            return error::candidate_error;
        }
    }

    if ((num_candidate_register == 0 && num_candidate_transfer == 0) || (num_candidate_register > 0 && num_candidate_transfer > 0))
    {
        log::debug(LOG_BLOCKCHAIN) << "candidate: illegal output.";
        return error::candidate_error;
    }

    // check inputs
    bool has_input_transfer = false;
    for (const auto &input : tx.inputs)
    {
        chain::transaction prev_tx;
        uint64_t prev_height{0};
        if (!get_previous_tx(prev_tx, prev_height, input))
        {
            log::debug(LOG_BLOCKCHAIN) << "candidate: input not found: "
                                       << encode_hash(input.previous_output.hash);
            return error::input_not_found;
        }

        auto prev_output = prev_tx.outputs.at(input.previous_output.index);
        if (prev_output.is_ucn())
        {
            auto &&token_address_in = prev_output.get_script_address();
            if (token_address != token_address_in)
            {
                log::debug(LOG_BLOCKCHAIN) << "candidate: invalid input address to pay fee: "
                                           << token_address_in << " != " << token_address;
                return error::validate_inputs_failed;
            }
        }
        else if (prev_output.is_candidate())
        {
            auto &&token_info = prev_output.get_candidate();
            if (token_symbol != token_info.get_symbol())
            {
                log::debug(LOG_BLOCKCHAIN) << "candidate: invalid candidate to transfer: "
                                           << token_info.get_symbol() << " != " << token_symbol;
                return error::validate_inputs_failed;
            }

            has_input_transfer = true;
        }
    }

    if (num_candidate_transfer > 0 && !has_input_transfer)
    {
        log::debug(LOG_BLOCKCHAIN) << "candidate: no input candidate to transfer " << token_symbol;
        return error::validate_inputs_failed;
    }

    return error::success;
}

bool validate_tx_engine::check_uid_exist(const std::string &uid) const
{
    uint64_t height = blockchain_.get_uid_height(uid);

    if (validate_block_)
    {
        //register before fork or find in orphan chain
        if (height <= validate_block_->get_fork_index() || validate_block_->is_uid_in_orphan_chain(uid))
        {
            return true;
        }

        return false;
    }

    return height != max_uint64;
}

bool validate_tx_engine::check_token_exist(const std::string &symbol) const
{
    uint64_t height = blockchain_.get_token_height(symbol);

    if (validate_block_)
    {
        //register before fork or find in orphan chain
        if (height <= validate_block_->get_fork_index() || validate_block_->is_token_in_orphan_chain(symbol))
        {
            return true;
        }

        return false;
    }

    return height != max_uint64;
}

bool validate_tx_engine::check_token_cert_exist(const std::string &cert, token_cert_type cert_type) const
{
    uint64_t height = blockchain_.get_token_cert_height(cert, cert_type);

    if (validate_block_)
    {
        //register before fork or find in orphan chain
        if (height <= validate_block_->get_fork_index() || validate_block_->is_token_cert_in_orphan_chain(cert, cert_type))
        {
            return true;
        }

        return false;
    }

    return height != max_uint64;
}

bool validate_tx_engine::check_candidate_exist(const std::string &candidate) const
{
    uint64_t height = blockchain_.get_candidate_height(candidate);

    if (validate_block_)
    {
        //register before fork or find in orphan chain
        if (height <= validate_block_->get_fork_index() || validate_block_->is_candidate_in_orphan_chain(candidate))
        {
            return true;
        }

        return false;
    }

    return height != max_uint64;
}

bool validate_tx_engine::check_address_registered_uid(const std::string &address) const
{
    uint64_t fork_index = validate_block_ ? validate_block_->get_fork_index() : max_uint64;
    auto uid_symbol = blockchain_.get_uid_from_address(address, fork_index);
    if (!validate_block_)
    {
        if (uid_symbol.empty())
        {
            return false;
        }
    }
    else
    {
        uid_symbol = validate_block_->get_uid_from_address_consider_orphan_chain(address, uid_symbol);
        if (uid_symbol.empty())
        {
            return false;
        }
    }

    log::debug(LOG_BLOCKCHAIN) << "address " << address << " already exists uid " << uid_symbol;
    return true;
}

code validate_tx_engine::check_uid_transaction() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;
    uint64_t fork_index = validate_block_ ? validate_block_->get_fork_index() : max_uint64;

    code ret = error::success;

    uint8_t type = 255;

    for (const auto &output : tx.outputs)
    {
        if ((ret = output.check_asset_address(chain)) != error::success)
            return ret;

        //to_uid check(strong check)
        if ((ret = check_asset_to_uid(output)) != error::success)
            return ret;

        //from_uid (weak check)
        if ((ret = connect_asset_from_uid(output)) != error::success)
        {
            return ret;
        }

        if (output.is_uid_register())
        {
            if (chain.is_valid_address(output.get_uid_symbol()))
            {
                return error::uid_symbol_invalid;
            }

            if (check_uid_exist(output.get_uid_symbol()))
            {
                log::debug(LOG_BLOCKCHAIN) << "uid_register: "
                                           << output.get_uid_symbol() << " already exists";
                return error::uid_exist;
            }

            if (check_address_registered_uid(output.get_uid_address()))
            {
                log::debug(LOG_BLOCKCHAIN) << "address "
                                           << output.get_uid_address() << " already exists uid, cannot register uid.";
                return error::address_registered_uid;
            }

            if (type != 255)
            {
                return error::uid_multi_type_exist;
            }
            type = UID_DETAIL_TYPE;

            if (!connect_uid_input(boost::get<uid>(output.get_uid())))
            {
                return error::uid_input_error;
            }
        }
        else if (output.is_uid_transfer())
        {
            if (check_address_registered_uid(output.get_uid_address()))
            {
                log::debug(LOG_BLOCKCHAIN) << "address "
                                           << output.get_uid_address() << " already exists uid, cannot transfer uid.";
                return error::address_registered_uid;
            }
            if (chain.exist_in_candidates(output.get_uid_symbol()))
                return error::uid_in_candidate;

            if (type != 255)
            {
                return error::uid_multi_type_exist;
            }
            type = UID_TRANSFERABLE_TYPE;

            if (!connect_uid_input(boost::get<uid>(output.get_uid())))
            {
                return error::uid_input_error;
            }
        }
        else if (output.is_token_issue() || output.is_token_secondaryissue())
        {
            if (output.attach_data.get_version() == UID_ASSET_VERIFY_VERSION && output.get_token_issuer() != output.attach_data.get_to_uid())
            {
                log::debug(LOG_BLOCKCHAIN)
                    << "token issuer " << output.get_token_issuer()
                    << " , does not match uid " << output.attach_data.get_to_uid()
                    << " , attach_data: " << output.attach_data.to_string();
                return error::token_uid_registerr_not_match;
            }
        }
        else if (output.is_token_cert())
        {
            if (output.attach_data.get_version() == UID_ASSET_VERIFY_VERSION)
            {
                if (output.get_token_cert_owner() != output.attach_data.get_to_uid())
                {
                    log::debug(LOG_BLOCKCHAIN)
                        << "cert owner " << output.get_token_cert_owner()
                        << " , does not match uid " << output.attach_data.get_to_uid()
                        << " , attach_data: " << output.attach_data.to_string();
                    return error::token_uid_registerr_not_match;
                }
            }
        }
        else if (output.is_vote() || output.is_candidate_transfer())
        {
            if (!output.is_uid_full_filled())
            {
                log::debug(LOG_BLOCKCHAIN)
                    << "both fromuid and touid are needed , attach_data: " << output.attach_data.to_string();
                return error::token_uid_registerr_not_match;
            }
        }
        else if (output.is_ucn_award())
        {
            if (!output.is_touid_filled())
            {
                log::debug(LOG_BLOCKCHAIN)
                    << "touid is needed , attach_data: " << output.attach_data.to_string();
                return error::token_uid_registerr_not_match;
            }
        }
        else if (output.is_candidate_register())
        {
            if (!output.is_fromuid_filled())
            {
                log::debug(LOG_BLOCKCHAIN)
                    << "fromuid is needed , attach_data: " << output.attach_data.to_string();
                return error::token_uid_registerr_not_match;
            }
            if (chain.exist_in_candidates(output.attach_data.get_from_uid()))
                return error::uid_in_candidate;
        }
    }

    return ret;
}

bool validate_tx_engine::connect_uid_input(const uid &info) const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;

    if (info.get_status() == UID_TRANSFERABLE_TYPE && tx.inputs.size() != 2)
    {
        return false;
    }

    auto detail_info = boost::get<uid_detail>(info.get_data());
    bool found_uid_info = false;
    bool found_address_info = false;

    for (const auto &input : tx.inputs)
    {
        chain::transaction prev_tx;
        uint64_t prev_height{0};
        if (!get_previous_tx(prev_tx, prev_height, input))
        {
            log::debug(LOG_BLOCKCHAIN) << "connect_uid_input: input not found: "
                                       << encode_hash(input.previous_output.hash);
            return false;
        }

        auto prev_output = prev_tx.outputs.at(input.previous_output.index);

        if (prev_output.is_uid_register() || prev_output.is_uid_transfer())
        {
            if (info.get_status() == UID_TRANSFERABLE_TYPE)
            {
                if (detail_info.get_symbol() == prev_output.get_uid_symbol())
                {
                    found_uid_info = true;
                }
            }
        }
        else if (prev_output.is_ucn())
        {
            auto uid_address_in = prev_output.get_script_address();
            if (detail_info.get_address() == uid_address_in)
            {
                found_address_info = true;
            }
        }
    }

    return (found_uid_info && found_address_info && info.get_status() == UID_TRANSFERABLE_TYPE) || (found_address_info && info.get_status() == UID_DETAIL_TYPE);
}

bool validate_tx_engine::is_uid_match_address_in_orphan_chain(const std::string &uid, const std::string &address) const
{
    if (validate_block_ && validate_block_->is_uid_match_address_in_orphan_chain(uid, address))
    {
        log::debug(LOG_BLOCKCHAIN) << "uid_in_orphan_chain: "
                                   << uid << ", match address: " << address;
        return true;
    }

    return false;
}

bool validate_tx_engine::is_uid_in_orphan_chain(const std::string &uid) const
{
    if (validate_block_ && validate_block_->is_uid_in_orphan_chain(uid))
    {
        log::debug(LOG_BLOCKCHAIN) << "uid_in_orphan_chain: " << uid << " exist";
        return true;
    }

    return false;
}

code validate_tx_engine::check_asset_to_uid(const output &output) const
{
    auto touid = output.attach_data.get_to_uid();
    if (touid.empty())
    {
        return error::success;
    }

    auto address = output.get_script_address();

    if (is_uid_match_address_in_orphan_chain(touid, address))
    {
        return error::success;
    }

    uint64_t fork_index = validate_block_ ? validate_block_->get_fork_index() : max_uint64;
    auto uid = blockchain_.get_uid_from_address(address, fork_index);
    if (touid == uid)
    {
        return error::success;
    }

    log::debug(LOG_BLOCKCHAIN) << "check_asset_to_uid: "
                               << touid << ", address: " << address
                               << "; get uid from address is " << uid;
    return error::uid_address_not_match;
}

code validate_tx_engine::connect_asset_from_uid(const output &output) const
{
    auto from_uid = output.attach_data.get_from_uid();
    if (from_uid.empty())
    {
        return error::success;
    }

    for (auto &input : tx_->inputs)
    {
        chain::transaction prev_tx;
        uint64_t prev_height{0};
        if (!get_previous_tx(prev_tx, prev_height, input))
        {
            log::debug(LOG_BLOCKCHAIN) << "connect_asset_from_uid: input not found: "
                                       << encode_hash(input.previous_output.hash);
            return error::input_not_found;
        }

        auto prev_output = prev_tx.outputs.at(input.previous_output.index);
        auto address = prev_output.get_script_address();

        if (is_uid_match_address_in_orphan_chain(from_uid, address))
        {
            return error::success;
        }

        uint64_t fork_index = validate_block_ ? validate_block_->get_fork_index() : max_uint64;
        if (from_uid == blockchain_.get_uid_from_address(address, fork_index))
        {
            return error::success;
        }
    }

    log::debug(LOG_BLOCKCHAIN) << "connect_asset_from_uid: input not found for from_uid: "
                               << from_uid;
    return error::uid_address_not_match;
}

code validate_tx_engine::check_transaction_connect_input(size_t last_height)
{
    if (last_height == 0 || tx_->is_strict_coinbase())
    {
        return error::success;
    }

    reset(last_height);

    for (const auto &input : tx_->inputs)
    {
        chain::transaction prev_tx;
        uint64_t prev_height{0};
        if (!get_previous_tx(prev_tx, prev_height, input))
        {
            log::debug(LOG_BLOCKCHAIN) << "check_transaction_connect_input: input not found: "
                                       << encode_hash(input.previous_output.hash);
            return error::input_not_found;
        }
        if (!connect_input(prev_tx, prev_height))
        {
            log::debug(LOG_BLOCKCHAIN) << "connect_input failed. prev height:"
                                       << std::to_string(prev_height)
                                       << ", prev hash: " << encode_hash(prev_tx.hash());
            return error::validate_inputs_failed;
        }
        ++current_input_;
    }

    return check_tx_connect_input();
}

code validate_tx_engine::check_transaction() const
{
    code ret = error::success;

    if ((ret = check_transaction_basic()) != error::success)
    {
        return ret;
    }

    if ((ret = check_token_issue_transaction()) != error::success)
    {
        return ret;
    }

    if (tx_->version >= transaction_version::check_uid_feature)
    {
        if ((ret = check_token_cert_transaction()) != error::success)
        {
            return ret;
        }

        if ((ret = check_secondaryissue_transaction()) != error::success)
        {
            return ret;
        }

        if ((ret = check_candidate_transaction()) != error::success)
        {
            return ret;
        }

        if ((ret = check_uid_transaction()) != error::success)
        {
            return ret;
        }

        if ((ret = attenuation_model::check_model_param(*this)) != error::success)
        {
            return ret;
        }
    }

    return ret;
}

code validate_tx_engine::check_transaction_basic() const
{
    const chain::transaction &tx = *tx_;
    blockchain::block_chain_impl &chain = blockchain_;

    if (tx.version >= transaction_version::max_version)
    {
        return error::transaction_version_error;
    }

    if (tx.version == transaction_version::check_uid_feature && !is_uid_feature_activated(chain))
    {
        return error::uid_feature_not_activated;
    }

    if (tx.version == transaction_version::check_uid_testnet && !chain.chain_settings().use_testnet_rules)
    {
        return error::transaction_version_error;
    }

    if (tx.version >= transaction_version::check_output_script)
    {
        for (auto &i : tx.outputs)
        {
            if (i.script.pattern() == script_pattern::non_standard)
            {
                return error::script_not_standard;
            }
        }
    }

    if (tx.inputs.empty() || tx.outputs.empty())
        return error::empty_transaction;

    if (tx.serialized_size() > max_transaction_size)
        return error::size_limits;

    // check double spend in inputs
    std::set<std::string> set;
    for (auto &input : tx.inputs)
    {
        auto tx_hash = libbitcoin::encode_hash(input.previous_output.hash);
        auto value = tx_hash + ":" + std::to_string(input.previous_output.index);
        if (set.count(value))
        {
            return error::double_spend;
        }

        set.insert(value);
    }

    // Check for negative or overflow output values
    uint64_t total_output_value = 0;

    for (const auto &output : tx.outputs)
    {
        if (output.value > max_money())
            return error::output_value_overflow;

        total_output_value += output.value;

        if (total_output_value > max_money())
            return error::output_value_overflow;
    }

    for (auto &output : tx.outputs)
    {
        if (output.is_token_issue())
        {
            if (!chain::output::is_valid_symbol(output.get_token_symbol(), tx.version))
            {
                return error::token_symbol_invalid;
            }
        }
        else if (output.is_token_cert())
        {
            if (!chain::output::is_valid_symbol(output.get_token_symbol(), tx.version))
            {
                return error::token_symbol_invalid;
            }
        }
        else if (output.is_uid_register())
        {
            auto is_test = chain.chain_settings().use_testnet_rules;
            if (!chain::output::is_valid_uid_symbol(output.get_uid_symbol(), !is_test))
            {
                return error::uid_symbol_invalid;
            }
        }
        else if (output.is_candidate_register())
        {
            if (!chain::output::is_valid_candidate_symbol(output.get_token_symbol(), true))
            {
                return error::candidate_symbol_invalid;
            }
        }

        // check asset, from uid version.
        if ((tx.version >= transaction_version::check_uid_feature) && (!output.attach_data.is_valid()))
        {
            log::debug(LOG_BLOCKCHAIN) << "invalid asset : "
                                       << output.attach_data.to_string();
            return error::asset_invalid;
        }
    }

    if (tx.is_coinbase())
    {
        const auto &coinbase_script = tx.inputs[0].script;
        const auto coinbase_size = coinbase_script.serialized_size(false);
        if (coinbase_size < 2 || coinbase_size > 200)
            return error::invalid_coinbase_script_size;
    }
    else
    {
        for (const auto &input : tx.inputs)
        {
            if (input.previous_output.is_null())
                return error::previous_output_null;

            if (chain::operation::is_sign_key_hash_with_lock_height_pattern(input.script.operations))
            {
                uint64_t prev_output_blockheight = 0;
                chain::transaction prev_tx;
                uint64_t current_blockheight = 0;

                chain.get_last_height(current_blockheight);
                if (!get_previous_tx(prev_tx, prev_output_blockheight, input))
                {
                    log::debug(LOG_BLOCKCHAIN) << "check_transaction_basic deposit : input not found: "
                                               << encode_hash(input.previous_output.hash);
                    return error::input_not_found;
                }

                uint64_t lock_height = chain::operation::get_lock_height_from_sign_key_hash_with_lock_height(input.script.operations);
                if (lock_height > current_blockheight - prev_output_blockheight)
                {
                    return error::invalid_input_script_lock_height;
                }
            }
        }

        for (auto &output : tx.outputs)
        {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations))
            {
                uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                if ((int)lock_height < 0
                    /*|| consensus::miner::get_lock_heights_index(lock_height) < 0*/)
                {
                    return error::invalid_output_script_lock_height;
                }
            }
        }
    }

    return error::success;
}

// Validate script consensus conformance based on flags provided.
bool validate_tx_engine::check_consensus(const script &prevout_script,
                                           const transaction &current_tx, size_t input_index, uint32_t flags)
{
    BITCOIN_ASSERT(input_index <= max_uint32);
    BITCOIN_ASSERT(input_index < current_tx.inputs.size());
    const auto input_index32 = static_cast<uint32_t>(input_index);

#ifdef WITH_CONSENSUS
    using namespace bc::consensus;
    const auto previous_output_script = prevout_script.to_data(false);
    data_chunk current_transaction = current_tx.to_data();

    // Convert native flags to libbitcoin-consensus flags.
    uint32_t consensus_flags = verify_flags_none;

    if ((flags & script_context::bip16_enabled) != 0)
        consensus_flags |= verify_flags_p2sh;

    if ((flags & script_context::bip65_enabled) != 0)
        consensus_flags |= verify_flags_checklocktimeverify;

    if ((flags & script_context::bip66_enabled) != 0)
        consensus_flags |= verify_flags_dersig;

    if ((flags & script_context::attenuation_enabled) != 0)
        consensus_flags |= verify_flags_checkattenuationverify;

    const auto result = verify_script(current_transaction.data(),
                                      current_transaction.size(), previous_output_script.data(),
                                      previous_output_script.size(), input_index32, consensus_flags);

    const auto valid = (result == verify_result::verify_result_eval_true);
#else
    // Copy the const prevout script so it can be run.
    auto previous_output_script = prevout_script;
    const auto &current_input_script = current_tx.inputs[input_index].script;

    const auto valid = script::verify(current_input_script,
                                      previous_output_script, current_tx, input_index32, flags);
#endif

    if (!valid)
    {
        log::warning(LOG_BLOCKCHAIN)
            << "Invalid transaction ["
            << encode_hash(current_tx.hash()) << "]";
    }

    return valid;
}

bool validate_tx_engine::connect_input(const transaction &previous_tx, size_t parent_height)
{
    const auto &input = tx_->inputs[current_input_];
    const auto &previous_outpoint = input.previous_output;

    if (previous_outpoint.index >= previous_tx.outputs.size())
    {
        log::debug(LOG_BLOCKCHAIN) << "output point index outof bounds!";
        return false;
    }

    const auto &previous_output = previous_tx.outputs[previous_outpoint.index];
    const auto output_value = previous_output.value;
    if (output_value > max_money())
    {
        log::debug(LOG_BLOCKCHAIN) << "output ucn value exceeds max amount!";
        return false;
    }

    token_cert_type token_certs = token_cert_ns::none;
    uint64_t token_transfer_amount = 0;
    if (previous_output.is_token())
    {
        auto new_symbol_in = previous_output.get_token_symbol();
        // 1. do token transfer amount check
        token_transfer_amount = previous_output.get_token_amount();

        // 2. do token symbol check
        if (!check_same(old_symbol_in_, new_symbol_in))
        {
            return false;
        }
        // check forbidden symbol
        if (bc::wallet::symbol::is_forbidden(new_symbol_in))
        {
            return false;
        }
    }
    else if (previous_output.is_token_cert())
    {
        token_certs = previous_output.get_token_cert_type();
        if (!check_same(old_cert_symbol_in_, previous_output.get_token_cert_symbol()))
        {
            return false;
        }
    }
    else if (previous_output.is_candidate())
    {
        if (!check_same(old_symbol_in_, previous_output.get_candidate_symbol()))
        {
            return false;
        }
    }
    else if (previous_output.is_uid())
    {
        // 1. do uid symbol check
        if (!check_same(old_symbol_in_, previous_output.get_uid_symbol()))
        {
            return false;
        }
    }

    /*if (previous_tx.is_coinbase()) {
        const auto height_difference = last_block_height_ - parent_height;
        if (height_difference < coinbase_maturity) {
            return false;
        }
    }*/

    if (!check_consensus(previous_output.script, *tx_, current_input_, script_context::all_enabled))
    {
        log::debug(LOG_BLOCKCHAIN) << "check_consensus failed";
        return false;
    }

    value_in_ += output_value;

    //for block token amount +1
    if (tx_->is_coinbase())
    {
        token_amount_in_ += (token_transfer_amount + 1);
    }
    else
    {
        token_amount_in_ += token_transfer_amount;
    }

    if (token_certs != token_cert_ns::none)
    {
        token_certs_in_.push_back(token_certs);
    }
    return value_in_ <= max_money();
}

bool validate_tx_engine::check_special_fees(bool is_testnet, const chain::transaction &tx, uint64_t fee)
{
    // check fee of issue token or register uid
    auto developer_community_address = bc::get_developer_community_address(is_testnet);
    std::vector<uint64_t> ucn_vec;
    uint32_t special_fee_type = 0;
    std::string to_address;
    for (auto &output : tx.outputs)
    {
        if (output.is_ucn())
        {
            if (output.get_script_address() == developer_community_address)
            {
                ucn_vec.push_back(output.value);
            }
            else
            {
                ucn_vec.push_back(0);
            }
        }
        else if (output.is_token_issue())
        {
            special_fee_type = 1;
            to_address = output.get_script_address();
        }
        else if (output.is_uid_register() || output.is_candidate_register())
        {
            special_fee_type = 2;
            to_address = output.get_script_address();
        }
    }

    // skip transaction to developer community address
    if (special_fee_type > 0 && to_address != developer_community_address)
    {
        uint64_t fee_to_developer = std::accumulate(ucn_vec.begin(), ucn_vec.end(), (uint64_t)0);
        if (fee_to_developer > 0)
        {
            uint64_t total_fee = fee + fee_to_developer;
            uint32_t percentage_to_miner = (uint32_t)std::ceil((fee * 100.0) / total_fee);

            bool result = false;
            if (special_fee_type == 1)
            {
                result = (total_fee >= bc::min_fee_to_issue_token && percentage_to_miner >= min_fee_percentage_to_miner);
            }
            else if (special_fee_type == 2)
            {
                result = (total_fee >= bc::min_fee_to_register_uid && percentage_to_miner >= min_fee_percentage_to_miner);
            }

            if (!result)
            {
                log::debug(LOG_BLOCKCHAIN) << "check fees failed: "
                                           << (special_fee_type == 1 ? "issue token" : "register uid")
                                           << ", total_fee: " << std::to_string(total_fee)
                                           << ", fee_percentage_to_miner: " << std::to_string(percentage_to_miner);
                return false;
            }
        }
        else
        {
            if (special_fee_type == 1)
            {
                if (fee < bc::min_fee_to_issue_token)
                {
                    log::debug(LOG_BLOCKCHAIN) << "check fees failed: fee for issuing token less than "
                                               << std::to_string(bc::min_fee_to_issue_token);
                    return false;
                }
            }
            else if (special_fee_type == 2)
            {
                if (fee < bc::min_fee_to_register_uid)
                {
                    log::debug(LOG_BLOCKCHAIN) << "check fees failed: fee for registerring uid less than "
                                               << std::to_string(bc::min_fee_to_register_uid);
                    return false;
                }
            }
        }
    }

    return true;
}

bool validate_tx_engine::tally_fees(blockchain::block_chain_impl &chain,
                                      const transaction &tx, uint64_t value_in, uint64_t &total_fees)
{
    const auto value_out = tx.total_output_value();

    if (tx.is_coinbase() && value_in == 0)
    {
        return true;
    }
    else
    {
        if (value_in < value_out)
            return false;

        const auto fee = value_in - value_out;

        if (fee < min_tx_fee)
        {
            return false;
        }

        total_fees += fee;
    }

    return total_fees <= max_money();
}

bool validate_tx_engine::check_token_amount(const transaction &tx) const
{
    const auto token_amount_out = tx.total_output_transfer_amount();
    if (token_amount_in_ != token_amount_out) // token amount must be equal
        return false;

    return true;
}

bool validate_tx_engine::check_token_symbol(const transaction &tx) const
{
    for (const auto &output : tx.outputs)
    {
        if (output.is_token())
        {
            if (old_symbol_in_ != output.get_token_symbol() && !output.is_vote())
            {
                return false;
            }
        }
        else if (output.is_token_cert())
        { // token cert related
            continue;
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            // token tx only related to token_cert and ucn output
            return false;
        }
    }
    return true;
}

bool validate_tx_engine::check_token_certs(const transaction &tx) const
{
    bool is_cert_transfer = false;
    bool is_cert_issue = false;
    bool has_cert_autoissue = false;
    bool has_token_issue = false;
    bool has_token_output = false;
    std::vector<token_cert_type> token_certs_out;
    for (auto &output : tx.outputs)
    {
        if (output.is_token_cert())
        {
            auto &&token_cert = output.get_token_cert();
            auto cert_type = token_cert.get_type();

            if (token_cert.get_status() == TOKEN_CERT_TRANSFER_TYPE)
            {
                is_cert_transfer = true;
            }
            else if (token_cert.get_status() == TOKEN_CERT_ISSUE_TYPE)
            {
                is_cert_issue = true;
            }
            else if (token_cert.get_status() == TOKEN_CERT_AUTOISSUE_TYPE)
            {
                has_cert_autoissue = true;
            }

            if (token_cert::test_certs(token_certs_out, cert_type))
            { // double certs exists
                return false;
            }

            // check token cert symbol
            if (token_cert::test_certs(token_certs_in_, token_cert_ns::domain) && (cert_type == token_cert_ns::naming))
            {
                auto &&domain = token_cert::get_domain(token_cert.get_symbol());
                if (domain != old_cert_symbol_in_)
                {
                    log::debug(LOG_BLOCKCHAIN) << "cert domain error: "
                                               << "symbol : " << token_cert.get_symbol() << "; "
                                               << old_cert_symbol_in_ << " != " << domain;
                    return false;
                }
            }
            else if (!token_cert.is_newly_generated())
            {
                if (old_cert_symbol_in_ != token_cert.get_symbol())
                {
                    log::debug(LOG_BLOCKCHAIN) << "cert symbol error: "
                                               << old_cert_symbol_in_ << " != " << token_cert.get_symbol();
                    return false;
                }
            }

            if (!token_cert.is_newly_generated())
            {
                if (!token_cert::test_certs(token_certs_in_, cert_type))
                {
                    log::debug(LOG_BLOCKCHAIN) << "input lack of cert: " << token_cert.get_type_name();
                    return false;
                }
            }

            token_certs_out.push_back(cert_type);
        }
        else if (output.is_token())
        { // token related
            has_token_output = true;
            if (output.is_token_issue())
            {
                has_token_issue = true;
            }
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            // token cert transfer tx only related to token_cert and ucn output
            log::debug(LOG_BLOCKCHAIN) << "cert tx mix other illegal output";
            return false;
        }
    }

    if ((is_cert_issue || is_cert_transfer) && has_token_output)
    {
        log::debug(LOG_BLOCKCHAIN) << "issue/transfer cert can not mix with token output";
        return false;
    }

    if (has_cert_autoissue && !has_token_issue)
    {
        log::debug(LOG_BLOCKCHAIN) << "only issue command can has cert with autoissue status";
        return false;
    }

    if (is_cert_transfer)
    {
        if (token_certs_in_.size() != 1)
        {
            log::debug(LOG_BLOCKCHAIN) << "transfer cert: invalid number of cert in inputs: "
                                       << token_certs_in_.size();
            return false;
        }
        if (token_certs_out.size() != 1)
        {
            log::debug(LOG_BLOCKCHAIN) << "transfer cert: invalid number of cert in outputs: "
                                       << token_certs_out.size();
            return false;
        }
    }

    if (!token_cert::test_certs(token_certs_out, token_certs_in_))
    {
        log::debug(LOG_BLOCKCHAIN) << "some inputed certs is missed in outputs";
        return false;
    }
    return true;
}

bool validate_tx_engine::check_candidate(const transaction &tx) const
{
    size_t num_candidate = 0;
    for (const auto &output : tx.outputs)
    {
        if (output.is_candidate_transfer())
        {
            if (++num_candidate > 1)
            {
                return false;
            }

            auto &&token_info = output.get_candidate();
            if (old_symbol_in_ != token_info.get_symbol())
            {
                return false;
            }
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            return false;
        }
    }

    return (num_candidate == 1);
}

bool validate_tx_engine::check_uid_symbol_match(const transaction &tx) const
{
    for (const auto &output : tx.outputs)
    {
        if (output.is_uid())
        {
            if (old_symbol_in_ != output.get_uid_symbol())
            {
                return false;
            }
        }
        else if (!output.is_ucn() && !output.is_message())
        {
            return false;
        }
    }
    return true;
}

bool validate_tx_engine::is_uid_feature_activated(blockchain::block_chain_impl &chain)
{
    /*if (chain.chain_settings().use_testnet_rules) {
        return true;
    }

    uint64_t current_blockheight = 0;
    chain.get_last_height(current_blockheight);

    // active SuperNove on 2018-06-18 (duanwu festival)
    return (current_blockheight > 1270000);*/
    return true;
}

} // namespace blockchain
} // namespace libbitcoin
