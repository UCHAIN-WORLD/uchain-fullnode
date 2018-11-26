/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
 *
 * UChain-explorer is free software: you can redistribute it and/or
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

#include <UChainService/api/command/base_helper.hpp>
#include <UChain/explorer/dispatch.hpp>
#include <UChainService/api/command/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using bc::chain::blockchain_message;
using bc::blockchain::validate_transaction;

utxo_attach_type get_utxo_attach_type(const chain::output& output_)
{
    auto& output = const_cast<chain::output&>(output_);
    if (output.is_ucn()) {
        return utxo_attach_type::ucn;
    }
    if (output.is_token_transfer()) {
        return utxo_attach_type::token_transfer;
    }
    if (output.is_token_issue()) {
        return utxo_attach_type::token_issue;
    }
    if (output.is_token_secondaryissue()) {
        return utxo_attach_type::token_secondaryissue;
    }
    if (output.is_token_cert()) {
        return utxo_attach_type::token_cert;
    }
    if (output.is_token_card()) {
        return utxo_attach_type::token_card;
    }
    if (output.is_uid_register()) {
        return utxo_attach_type::uid_register;
    }
    if (output.is_uid_transfer()) {
        return utxo_attach_type::uid_transfer;
    }
    if (output.is_message()) {
        return utxo_attach_type::message;
    }
    if (output.is_ucn_award()) {
        throw std::logic_error("get_utxo_attach_type : Unexpected ucn_award type.");
    }
    throw std::logic_error("get_utxo_attach_type : Unkown output type "
            + std::to_string(output.attach_data.get_type()));
}

void check_uid_symbol(const std::string& symbol, bool check_sensitive)
{
    if (!chain::output::is_valid_uid_symbol(symbol, check_sensitive)) {
        throw uid_symbol_name_exception{"Did symbol " + symbol + " is not valid."};
    }

    if (check_sensitive) {
        if (boost::iequals(symbol, "BLACKHOLE")) {
            throw uid_symbol_name_exception{"Did symbol cannot be blackhole."};
        }
    }
}

void check_token_symbol(const std::string& symbol, bool check_sensitive)
{
    if (symbol.empty()) {
        throw token_symbol_length_exception{"Symbol cannot be empty."};
    }

    if (symbol.length() > TOKEN_DETAIL_SYMBOL_FIX_SIZE) {
        throw token_symbol_length_exception{"Symbol length must be less than "
            + std::to_string(TOKEN_DETAIL_SYMBOL_FIX_SIZE) + "."};
    }

    if (check_sensitive) {
        if (bc::wallet::symbol::is_sensitive(symbol)) {
            throw token_symbol_name_exception{"Symbol " + symbol + " is forbidden."};
        }
    }
}

void check_card_symbol(const std::string& symbol, bool check_sensitive)
{
    if (symbol.empty()) {
        throw token_symbol_length_exception{"Symbol cannot be empty."};
    }

    if (symbol.length() > TOKEN_CARD_SYMBOL_FIX_SIZE) {
        throw token_symbol_length_exception{"Symbol length must be less than "
            + std::to_string(TOKEN_CARD_SYMBOL_FIX_SIZE) + "."};
    }

    // char check
    for (const auto& i : symbol) {
        if (!(std::isalnum(i) || i == '.'|| i == '@' || i == '_' || i == '-'))
            throw token_symbol_name_exception(
                "Symbol " + symbol + " has invalid character.");
    }

    if (check_sensitive) {
        auto upper = boost::to_upper_copy(symbol);
        if (bc::wallet::symbol::is_sensitive(upper)) {
            throw token_symbol_name_exception{"Symbol " + symbol + " is forbidden."};
        }
    }
}

std::string get_address(const std::string& uid_or_address,
    bc::blockchain::block_chain_impl& blockchain)
{
    std::string address;
    if (!uid_or_address.empty()) {
        if (blockchain.is_valid_address(uid_or_address)) {
            address = uid_or_address;
        }
        else {
            address = get_address_from_uid(uid_or_address, blockchain);
        }
    }
    return address;
}

std::string get_address(const std::string& uid_or_address,
    asset& attach, bool is_from,
    bc::blockchain::block_chain_impl& blockchain)
{
    std::string address;
    if (blockchain.is_valid_address(uid_or_address)) {
        address = uid_or_address;
    }
    else {
        address = get_address_from_uid(uid_or_address, blockchain);
        if (is_from) {
            attach.set_from_uid(uid_or_address);
        }
        else {
            attach.set_to_uid(uid_or_address);
        }
        attach.set_version(UID_ASSET_VERIFY_VERSION);
    }
    return address;
}

std::string get_address_from_uid(const std::string& uid,
    bc::blockchain::block_chain_impl& blockchain)
{
    check_uid_symbol(uid);

    auto uiddetail = blockchain.get_registered_uid(uid);
    if (!uiddetail) {
        throw uid_symbol_notfound_exception{"uid " + uid + " does not exist on the blockchain"};
    }
    return uiddetail->get_address();
}

std::string get_random_payment_address(
    std::shared_ptr<account_address::list> sp_addresses,
    bc::blockchain::block_chain_impl& blockchain)
{
    if (sp_addresses && !sp_addresses->empty()) {
        // first, let test 10 times of random
        for (auto i = 0; i < 10; ++i) {
            auto random = bc::pseudo_random();
            auto index = random % sp_addresses->size();
            auto addr = sp_addresses->at(index).get_address();
            if (blockchain.is_payment_address(addr)) {
                return addr;
            }
        }
        // then, real bad luck, lets filter only the payment address
        account_address::list filtered_addresses;
        std::copy_if(sp_addresses->begin(), sp_addresses->end(),
            std::back_inserter(filtered_addresses),
            [&blockchain](const auto& each){
               return blockchain.is_payment_address(each.get_address());
        });

        if (!filtered_addresses.empty()) {
            auto random = bc::pseudo_random();
            auto index = random % filtered_addresses.size();
            return filtered_addresses.at(index).get_address();
        }
    }
    return "";
}

void sync_fetch_token_cert_balance(const std::string& address, const string& symbol,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_cert::list> sh_vec,
    token_cert_type cert_type)
{
    chain::transaction tx_temp;
    uint64_t tx_height;

    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));
    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address) {
                continue;
            }
            if (output.is_token_cert())
            {
                auto token_cert = output.get_token_cert();
                if (!symbol.empty() && symbol != token_cert.get_symbol()) {
                    continue;
                }
                if (cert_type != token_cert_ns::none && cert_type != token_cert.get_type()) {
                    continue;
                }

                sh_vec->push_back(std::move(token_cert));
            }
        }
    }
}

void sync_fetch_token_balance(const std::string& address, bool sum_all,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_balances::list> sh_token_vec)
{
    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address) {
                continue;
            }
            if (output.is_token())
            {
                const auto& symbol = output.get_token_symbol();
                if (bc::wallet::symbol::is_forbidden(symbol)) {
                    // swallow forbidden symbol
                    continue;
                }

                auto match = [sum_all, &symbol, &address](const token_balances& elem) {
                    return (symbol == elem.symbol) && (sum_all || (address == elem.address));
                };
                auto iter = std::find_if(sh_token_vec->begin(), sh_token_vec->end(), match);

                auto token_amount = output.get_token_amount();
                uint64_t locked_amount = 0;
                if (token_amount
                    && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    const auto& attenuation_model_param = output.get_attenuation_model_param();
                    auto diff_height = row.output_height ? (height - row.output_height) : 0;
                    auto available_amount = attenuation_model::get_available_token_amount(
                            token_amount, diff_height, attenuation_model_param);
                    locked_amount = token_amount - available_amount;
                }
                if (iter == sh_token_vec->end()) { // new item
                    sh_token_vec->push_back({symbol, address, token_amount, locked_amount});
                }
                else { // exist just add amount
                    iter->unspent_token += token_amount;
                    iter->locked_token += locked_amount;
                }
            }
        }
    }
}

void sync_fetch_token_deposited_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_deposited_balance::list> sh_token_vec)
{
    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.is_token())
            {
                if (!operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    continue;
                }

                auto token_amount = output.get_token_amount();
                if (token_amount == 0) {
                    continue;
                }

                const auto& symbol = output.get_token_symbol();
                if (bc::wallet::symbol::is_forbidden(symbol)) {
                    // swallow forbidden symbol
                    continue;
                }

                const auto& model_param = output.get_attenuation_model_param();
                auto diff_height = row.output_height ? (height - row.output_height) : 0;
                auto available_amount = attenuation_model::get_available_token_amount(
                        token_amount, diff_height, model_param);
                uint64_t locked_amount = token_amount - available_amount;
                if (locked_amount == 0) {
                    continue;
                }

                token_deposited_balance deposited(
                    symbol, address, encode_hash(row.output.hash), row.output_height);
                deposited.unspent_token = token_amount;
                deposited.locked_token = locked_amount;
                deposited.model_param = std::string(model_param.begin(), model_param.end());
                sh_token_vec->push_back(deposited);
            }
        }
    }
}

void sync_unspend_output(bc::blockchain::block_chain_impl& blockchain, const input_point& input,
 std::shared_ptr<output_point::list>& output_list,  base_transfer_common::filter filter)
{
    auto is_filter = [filter](const output & output_){
        if (((filter & base_transfer_common::FILTER_UCN) && output_.is_ucn())
        || ( (filter & base_transfer_common::FILTER_TOKEN) && output_.is_token())
        || ( (filter & base_transfer_common::FILTER_IDENTIFIABLE_TOKEN) && output_.is_token_card())
        || ( (filter & base_transfer_common::FILTER_TOKENCERT) && output_.is_token_cert())
        || ( (filter & base_transfer_common::FILTER_UID) && output_.is_uid())){
            return true;
        }
        return false;
    };

    std::shared_ptr<chain::transaction> tx = blockchain.get_spends_output(input);
    uint64_t tx_height;
    chain::transaction tx_temp;
    if(tx == nullptr && blockchain.get_transaction(input.hash, tx_temp, tx_height))
    {
        const auto& output = tx_temp.outputs.at(input.index);

        if (is_filter(output)){
            output_list->emplace_back(input);
        }
    }
    else if (tx != nullptr)
    {

        for (uint32_t i = 0; i < tx->outputs.size(); i++)
        {
            const auto& output = tx->outputs.at(i);
            if (is_filter(output)){
                input_point input_ = {tx->hash(), i};
                sync_unspend_output(blockchain, input_, output_list, filter);
            }

        }

    }

}

auto get_token_unspend_utxo(const std::string& symbol,
 bc::blockchain::block_chain_impl& blockchain) -> std::shared_ptr<output_point::list>
{
    auto blockchain_tokens = blockchain.get_token_register_output(symbol);
    if (blockchain_tokens == nullptr || blockchain_tokens->empty()){
        throw token_symbol_existed_exception(std::string("token symbol[") +symbol + "]does not exist!");
    }

    std::shared_ptr<output_point::list> output_list = std::make_shared<output_point::list>();
    for (auto token : *blockchain_tokens)
    {
        auto out_point = token.get_tx_point();
        sync_unspend_output(blockchain, out_point, output_list, base_transfer_common::FILTER_TOKEN);
    }
    if(!output_list->empty()){
        std::sort(output_list->begin(), output_list->end());
        output_list->erase(std::unique(output_list->begin(), output_list->end()), output_list->end());
    }
    return output_list;
}

auto sync_fetch_token_deposited_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain)
     -> std::shared_ptr<token_deposited_balance::list>
{

    std::shared_ptr<output_point::list> output_list = get_token_unspend_utxo(symbol, blockchain);

    std::shared_ptr<token_deposited_balance::list> sh_token_vec = std::make_shared<token_deposited_balance::list>();

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto &out : *output_list)
    {
        // spend unconfirmed (or no spend attempted)
        if (blockchain.get_transaction(out.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(out.index < tx_temp.outputs.size());
            const auto &output = tx_temp.outputs.at(out.index);
            if (output.is_token())
            {
                std::string address = output.get_script_address();

                const auto &symbol = output.get_token_symbol();
                if (output.get_token_symbol() != symbol ||
                    bc::wallet::symbol::is_forbidden(symbol))
                {
                    // swallow forbidden symbol
                    continue;
                }

                if (!operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations))
                {
                    continue;
                }

                auto token_amount = output.get_token_amount();
                if (token_amount == 0)
                {
                    continue;
                }

                const auto &model_param = output.get_attenuation_model_param();
                auto diff_height = tx_height ? (height - tx_height) : 0;
                auto available_amount = attenuation_model::get_available_token_amount(
                    token_amount, diff_height, model_param);
                uint64_t locked_amount = token_amount - available_amount;
                if (locked_amount == 0)
                {
                    continue;
                }

                token_deposited_balance deposited(
                    symbol, address, encode_hash(out.hash), tx_height);
                deposited.unspent_token = token_amount;
                deposited.locked_token = locked_amount;
                deposited.model_param = std::string(model_param.begin(), model_param.end());
                sh_token_vec->emplace_back(deposited);
            }
        }
    }

    return sh_token_vec;
}


auto sync_fetch_token_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain)
     -> std::shared_ptr<token_balances::list>
{

    std::shared_ptr<output_point::list> output_list = get_token_unspend_utxo(symbol, blockchain);

    std::shared_ptr<token_balances::list> sh_token_vec = std::make_shared<token_balances::list>();

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto &out : *output_list)
    {
        // spend unconfirmed (or no spend attempted)
        if (blockchain.get_transaction(out.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(out.index < tx_temp.outputs.size());
            const auto &output = tx_temp.outputs.at(out.index);
            if (output.is_token())
            {
                std::string address = output.get_script_address();

                const auto &symbol = output.get_token_symbol();
                if (output.get_token_symbol() != symbol ||
                    bc::wallet::symbol::is_forbidden(symbol))
                {
                    // swallow forbidden symbol
                    continue;
                }


                auto token_amount = output.get_token_amount();
                uint64_t locked_amount = 0;
                if (token_amount
                    && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    const auto& attenuation_model_param = output.get_attenuation_model_param();
                    auto diff_height = tx_height ? (height - tx_height) : 0;
                    auto available_amount = attenuation_model::get_available_token_amount(
                            token_amount, diff_height, attenuation_model_param);
                    locked_amount = token_amount - available_amount;
                }

                sh_token_vec->push_back({symbol, address, token_amount, locked_amount});

            }
        }
    }

    return sh_token_vec;
}

static uint32_t get_domain_cert_count(bc::blockchain::block_chain_impl& blockchain,
    const std::string& account_name)
{
    auto pvaddr = blockchain.get_account_addresses(account_name);
    if (!pvaddr) {
        return 0;
    }

    auto sh_vec = std::make_shared<token_cert::list>();
    for (auto& each : *pvaddr){
        sync_fetch_token_cert_balance(each.get_address(), "", blockchain, sh_vec, token_cert_ns::domain);
    }

    return sh_vec->size();
}

void sync_fetch_deposited_balance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<deposited_balance::list> sh_vec)
{
    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    auto&& address_str = address.encoded();
    auto&& rows = blockchain.get_address_history(address, false);
    for (auto& row: rows) {
        if (row.output_height == 0) {
            continue;
        }

        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height)) {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            auto output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address.encoded()) {
                continue;
            }

            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                // deposit utxo in block
                uint64_t deposit_height = chain::operation::
                    get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                uint64_t expiration_height = row.output_height + deposit_height;

                if (expiration_height > height) {
                    auto&& output_hash = encode_hash(row.output.hash);
                    auto&& tx_hash = encode_hash(tx_temp.hash());
                    const auto match = [&tx_hash](const deposited_balance& balance) {
                        return balance.tx_hash == tx_hash;
                    };
                    auto iter = std::find_if(sh_vec->begin(), sh_vec->end(), match);
                    if (iter != sh_vec->end()) {
                        if (output_hash == tx_hash) {
                            iter->balance = row.value;
                        }
                        else {
                            iter->bonus = row.value;
                            iter->bonus_hash = output_hash;
                        }
                    }
                    else {

                        deposited_balance deposited(address_str, tx_hash, deposit_height, expiration_height);
                        if (output_hash == tx_hash) {
                            deposited.balance = row.value;
                        }
                        else {
                            deposited.bonus = row.value;
                            deposited.bonus_hash = output_hash;
                        }
                        sh_vec->push_back(std::move(deposited));
                    }
                }
            }
        }
    }
}

void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance)
{
    auto&& rows = blockchain.get_address_history(address, false);

    uint64_t total_received = 0;
    uint64_t confirmed_balance = 0;
    uint64_t unspent_balance = 0;
    uint64_t frozen_balance = 0;

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows) {
        /*if (row.output_height == 0) {
            continue;
        }*/

        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height)) {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            auto output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address.encoded()) {
                continue;
            }

            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                // deposit utxo in block
                uint64_t lock_height = chain::operation::
                    get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                if ((row.output_height + lock_height) > height) {
                    // utxo already in block but deposit not expire
                    frozen_balance += row.value;
                }
            }
            else if (tx_temp.is_coinbase()) { // coin base ucn maturity ucn check
                // add not coinbase_maturity ucn into frozen
                if ((row.output_height + coinbase_maturity) > height) {
                    frozen_balance += row.value;
                }
            }

            unspent_balance += row.value;
        }

        total_received += row.value;

        if ((row.spend.hash == null_hash || row.spend_height == 0))
            confirmed_balance += row.value;
    }

    addr_balance.confirmed_balance = confirmed_balance;
    addr_balance.total_received = total_received;
    addr_balance.unspent_balance = unspent_balance;
    addr_balance.frozen_balance = frozen_balance;
}

bool base_transfer_common::get_spendable_output(
    chain::output& output, const chain::history& row, uint64_t height) const
{
    // spended
    if (row.spend.hash != null_hash) {
        return false;
    }

    chain::transaction tx_temp;
    uint64_t tx_height;
    if (!blockchain_.get_transaction(row.output.hash, tx_temp, tx_height)) {
        return false;
    }

    BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
    output = tx_temp.outputs.at(row.output.index);

    if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
        if (row.output_height == 0) {
            // deposit utxo in transaction pool
            return false;
        } else {
            // deposit utxo in block
            auto lock_height = chain::operation::
                get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
            if ((row.output_height + lock_height) > height) {
                // utxo already in block but deposit not expire
                return false;
            }
        }
    } //else if (tx_temp.is_coinbase()) { // incase readd deposit
        // coin base ucn maturity ucn check
        // coinbase_maturity ucn check
        //if (/*(row.output_height == 0) ||*/ ((row.output_height + coinbase_maturity) > height)) {
        //    return false;
        //}
    //}

    return true;
}

// only consider ucn and token and cert.
// specify parameter 'uid' to true to only consider uid
void base_transfer_common::sync_fetchutxo(
        const std::string& prikey, const std::string& addr, filter filter)
{
    auto&& waddr = wallet::payment_address(addr);
    auto&& rows = blockchain_.get_address_history(waddr, true);

    uint64_t height = 0;
    blockchain_.get_last_height(height);

    for (auto& row: rows)
    {
        // performance improve
        if (is_payment_satisfied(filter)) {
            break;
        }

        chain::output output;
        if (!get_spendable_output(output, row, height)) {
            continue;
        }

        if (output.get_script_address() != addr) {
            continue;
        }

        auto ucn_amount = row.value;
        auto token_total_amount = output.get_token_amount();
        auto cert_type = output.get_token_cert_type();
        auto token_symbol = output.get_token_symbol();

        // filter output
        if ((filter & FILTER_UCN) && output.is_ucn()) { // ucn related
            BITCOIN_ASSERT(token_total_amount == 0);
            BITCOIN_ASSERT(token_symbol.empty());
            if (ucn_amount == 0)
                continue;
            // enough ucn to pay
            if (unspent_ucn_ >= payment_ucn_)
                continue;
        }
        else if ((filter & FILTER_TOKEN) && output.is_token()) { // token related
            BITCOIN_ASSERT(ucn_amount == 0);
            BITCOIN_ASSERT(cert_type == token_cert_ns::none);
            if (token_total_amount == 0)
                continue;
            // enough token to pay
            if (unspent_token_ >= payment_token_)
                continue;
            // check token symbol
            if (symbol_ != token_symbol)
                continue;

            if (bc::wallet::symbol::is_forbidden(token_symbol)) {
                // swallow forbidden symbol
                continue;
            }
        }
        else if ((filter & FILTER_IDENTIFIABLE_TOKEN) && output.is_token_card()) {
            BITCOIN_ASSERT(ucn_amount == 0);
            BITCOIN_ASSERT(token_total_amount == 0);
            BITCOIN_ASSERT(cert_type == token_cert_ns::none);

            if (payment_card_ <= unspent_card_) {
                continue;
            }

            if (symbol_ != output.get_token_symbol())
                continue;

            ++unspent_card_;
        }
        else if ((filter & FILTER_TOKENCERT) && output.is_token_cert()) { // cert related
            BITCOIN_ASSERT(ucn_amount == 0);
            BITCOIN_ASSERT(token_total_amount == 0);
            // no needed token cert is included in this output
            if (payment_token_cert_.empty())
                continue;

            // check cert symbol
            if (cert_type == token_cert_ns::domain) {
                auto&& domain = token_cert::get_domain(symbol_);
                if (domain != token_symbol)
                    continue;
            }
            else {
                if (symbol_ != token_symbol)
                    continue;
            }

            // check cert type
            if (!token_cert::test_certs(payment_token_cert_, cert_type)) {
                continue;
            }

            // token cert has already found
            if (token_cert::test_certs(unspent_token_cert_, payment_token_cert_)) {
                continue;
            }
        }
        else if ((filter & FILTER_UID) &&
            (output.is_uid_register() || output.is_uid_transfer())) { // uid related
            BITCOIN_ASSERT(ucn_amount == 0);
            BITCOIN_ASSERT(token_total_amount == 0);
            BITCOIN_ASSERT(cert_type == token_cert_ns::none);

            if (payment_uid_ <= unspent_uid_) {
                continue;
            }

            if (symbol_ != output.get_uid_symbol())
                continue;

            ++unspent_uid_;
        }
        else {
            continue;
        }

        auto token_amount = token_total_amount;
        std::shared_ptr<data_chunk> new_model_param_ptr;
        if (token_total_amount
            && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
            const auto& attenuation_model_param = output.get_attenuation_model_param();
            new_model_param_ptr = std::make_shared<data_chunk>();
            auto diff_height = row.output_height ? (height - row.output_height) : 0;
            token_amount = attenuation_model::get_available_token_amount(
                    token_total_amount, diff_height, attenuation_model_param, new_model_param_ptr);
            if ((token_amount == 0) && !is_locked_token_as_payment()) {
                continue; // all locked, filter out
            }
        }

        BITCOIN_ASSERT(token_total_amount >= token_amount);

        // add to from list
        address_token_record record;

        if (!prikey.empty()) { // raw tx has no prikey
            record.prikey = prikey;
            record.script = output.script;
        }
        record.addr = addr;
        record.amount = ucn_amount;
        record.symbol = token_symbol;
        record.token_amount = token_amount;
        record.token_cert = cert_type;
        record.output = row.output;
        record.type = get_utxo_attach_type(output);

        from_list_.push_back(record);

        unspent_ucn_ += record.amount;
        unspent_token_ += record.token_amount;

        if (record.token_cert != token_cert_ns::none) {
            unspent_token_cert_.push_back(record.token_cert);
        }

        // token_locked_transfer as a special change
        if (new_model_param_ptr && (token_total_amount > record.token_amount)) {
            auto locked_token = token_total_amount - record.token_amount;
            std::string model_param(new_model_param_ptr->begin(), new_model_param_ptr->end());
            receiver_list_.push_back({record.addr, record.symbol,
                    0, locked_token, utxo_attach_type::token_locked_transfer,
                    asset(0, 0, blockchain_message(std::move(model_param))), record.output});
            // in secondary issue, locked token can also verify threshold condition
            if (is_locked_token_as_payment()) {
                payment_token_ = (payment_token_ > locked_token)
                    ? (payment_token_ - locked_token) : 0;
            }
        }
    }

    rows.clear();
}

void base_transfer_common::check_fee_in_valid_range(uint64_t fee)
{
    if ((fee < minimum_fee) || (fee > maximum_fee)) {
        throw token_exchange_poundage_exception{"fee must in ["
            + std::to_string(minimum_fee) + ", " + std::to_string(maximum_fee) + "]"};
    }
}

void base_transfer_common::check_model_param_initial(std::string& param, uint64_t amount)
{
    if (!param.empty()) {
        if (!validate_transaction::is_nova_feature_activated(blockchain_)) {
            throw token_attenuation_model_exception(
                "attenuation model should be supported after nova feature is activated.");
        }
        if (!attenuation_model::check_model_param_initial(param, amount, true)) {
            throw token_attenuation_model_exception("check token attenuation model param failed");
        }
    }
}

void base_transfer_common::sum_payments()
{
    for (auto& iter : receiver_list_) {
        payment_ucn_ += iter.amount;
        //vote token will be not counted
        if (symbol_ != UC_VOTE_TOKEN_SYMBOL) {
            payment_token_ += iter.token_amount;
        }       

        if (iter.token_cert != token_cert_ns::none) {
            payment_token_cert_.push_back(iter.token_cert);
        }

        if (iter.type == utxo_attach_type::token_card_transfer) {
            ++payment_card_;
            if (payment_card_ > 1) {
                throw std::logic_error{"maximum one card can be transfered"};
            }
        }
        else if (iter.type == utxo_attach_type::uid_transfer) {
            ++payment_uid_;
            if (payment_uid_ > 1) {
                throw std::logic_error{"maximum one UID can be transfered"};
            }
        }
    }
}

void base_transfer_common::check_receiver_list_not_empty() const
{
    if (receiver_list_.empty()) {
        throw toaddress_empty_exception{"empty target address"};
    }
}

void base_transfer_common::sum_payment_amount()
{
    check_receiver_list_not_empty();
    check_fee_in_valid_range(payment_ucn_);
    sum_payments();
}

bool base_transfer_common::is_payment_satisfied(filter filter) const
{
    if ((filter & FILTER_UCN) && (unspent_ucn_ < payment_ucn_))
        return false;

    if ((filter & FILTER_TOKEN) && (unspent_token_ < payment_token_))
        return false;

    if ((filter & FILTER_IDENTIFIABLE_TOKEN) && (unspent_card_ < payment_card_))
        return false;

    if ((filter & FILTER_TOKENCERT)
        && !token_cert::test_certs(unspent_token_cert_, payment_token_cert_))
        return false;

    if ((filter & FILTER_UID) && (unspent_uid_ < payment_uid_))
        return false;

    return true;
}

void base_transfer_common::check_payment_satisfied(filter filter) const
{
    if ((filter & FILTER_UCN) && (unspent_ucn_ < payment_ucn_)) {
        throw account_balance_lack_exception{"not enough balance, unspent = "
            + std::to_string(unspent_ucn_) + ", payment = " + std::to_string(payment_ucn_)};
    }

    if ((filter & FILTER_TOKEN) && (unspent_token_ < payment_token_) && symbol_ != UC_VOTE_TOKEN_SYMBOL) {
        throw token_lack_exception{"not enough token amount, unspent = "
            + std::to_string(unspent_token_) + ", payment = " + std::to_string(payment_token_)};
    }

    if ((filter & FILTER_IDENTIFIABLE_TOKEN) && (unspent_card_ < payment_card_)) {
        throw token_lack_exception{"not enough card amount, unspent = "
            + std::to_string(unspent_card_) + ", payment = " + std::to_string(payment_card_)};
    }

    if ((filter & FILTER_TOKENCERT)
        && !token_cert::test_certs(unspent_token_cert_, payment_token_cert_)) {
        std::string payment(" ");
        for (auto& cert_type : payment_token_cert_) {
            payment += token_cert::get_type_name(cert_type);
            payment += " ";
        }
        std::string unspent(" ");
        for (auto& cert_type : unspent_token_cert_) {
            unspent += token_cert::get_type_name(cert_type);
            unspent += " ";
        }

        throw token_cert_exception{"not enough token cert, unspent = ("
            + unspent + "), payment = (" + payment + ")"};
    }

    if ((filter & FILTER_UID) && (unspent_uid_ < payment_uid_)) {
        throw tx_source_exception{"no uid named " + symbol_ + " is found"};
    }
}

void base_transfer_common::populate_change()
{
    // only ucn utxo, others in derived class
    populate_ucn_change();
}

std::string base_transfer_common::get_mychange_address(filter filter) const
{
    if (!mychange_.empty()) {
        return mychange_;
    }

    if ((filter & FILTER_PAYFROM) && !from_.empty()) {
        return from_;
    }

    const auto match = [filter](const address_token_record& record) {
        if (filter & FILTER_UCN) {
            return (record.type == utxo_attach_type::ucn);
        }
        if (filter & FILTER_TOKEN) {
            return (record.type == utxo_attach_type::token_transfer)
                || (record.type == utxo_attach_type::token_issue)
                || (record.type == utxo_attach_type::token_secondaryissue);
        }
        throw std::logic_error{"get_mychange_address: unknown/wrong filter for mychange"};
    };

    // reverse find the lates matched unspent
    auto it = from_list_.crbegin();
    for (; it != from_list_.crend(); ++it) {
        if (match(*it)) {
            return it->addr;
        }
    }
    BITCOIN_ASSERT(it != from_list_.crend());

    return from_list_.begin()->addr;
}

void base_transfer_common::populate_ucn_change(const std::string& address)
{
    // ucn utxo
    if (unspent_ucn_ > payment_ucn_) {
        auto addr = address;
        if (addr.empty()) {
            addr = get_mychange_address(FILTER_UCN);
        }
        BITCOIN_ASSERT(!addr.empty());

        if (blockchain_.is_valid_address(addr)) {
            receiver_list_.push_back(
                {addr, "", unspent_ucn_ - payment_ucn_, 0, utxo_attach_type::ucn, asset()});
        }
        else {
            if (addr.length() > UID_DETAIL_SYMBOL_FIX_SIZE) {
                throw uid_symbol_length_exception{
                    "mychange uid symbol " + addr + " length must be less than 64."};
            }

            auto uiddetail = blockchain_.get_registered_uid(addr);
            if (!uiddetail) {
                throw uid_symbol_notfound_exception{
                    "mychange uid symbol " + addr + "does not exist on the blockchain"};
            }

            asset attach;
            attach.set_version(UID_ASSET_VERIFY_VERSION);
            attach.set_to_uid(addr);
            receiver_list_.push_back(
                {uiddetail->get_address(), "", unspent_ucn_ - payment_ucn_, 0, utxo_attach_type::ucn, attach});
        }
    }
}

void base_transfer_common::populate_token_change(const std::string& address)
{
    // token utxo
    if (unspent_token_ > payment_token_) {
        auto addr = address;
        if (addr.empty()) {
            addr = get_mychange_address(FILTER_TOKEN);
        }
        BITCOIN_ASSERT(!addr.empty());

        if (blockchain_.is_valid_address(addr)) {
            receiver_list_.push_back({addr, symbol_, 0, unspent_token_ - payment_token_,
                utxo_attach_type::token_transfer, asset()});
        }
        else {
            if (addr.length() > UID_DETAIL_SYMBOL_FIX_SIZE) {
                throw uid_symbol_length_exception{
                    "mychange uid symbol " + addr + " length must be less than 64."};
            }

            auto uiddetail = blockchain_.get_registered_uid(addr);
            if (!uiddetail) {
                throw uid_symbol_notfound_exception{
                    "mychange uid symbol " + addr + "does not exist on the blockchain"};
            }

            asset attach;
            attach.set_version(UID_ASSET_VERIFY_VERSION);
            attach.set_to_uid(addr);
            receiver_list_.push_back({uiddetail->get_address(), symbol_, 0, unspent_token_ - payment_token_,
                utxo_attach_type::token_transfer, attach});
        }
    }
}

chain::operation::stack
base_transfer_common::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and token should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    const auto& hash = payment.hash();
    if (blockchain_.is_blackhole_address(record.target)) {
        payment_ops = chain::operation::to_pay_blackhole_pattern(hash);
    }
    else if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        if (record.type == utxo_attach_type::token_locked_transfer) { // for token locked change only
            const auto& attenuation_model_param =
                boost::get<blockchain_message>(record.attach_elem.get_attach()).get_content();
            if (!attenuation_model::check_model_param_format(to_chunk(attenuation_model_param))) {
                throw token_attenuation_model_exception(
                    "check token locked transfer attenuation model param failed: "
                    + attenuation_model_param);
            }
            payment_ops = chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                hash, attenuation_model_param, record.input_point);
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash);
        }
    }
    else if (payment.version() == wallet::payment_address::mainnet_p2sh) {
        payment_ops = chain::operation::to_pay_script_hash_pattern(hash);
    }
    else {
        throw toaddress_unrecognized_exception{"unrecognized target address : " + payment.encoded()};
    }

    return payment_ops;
}

chain::operation::stack
base_transfer_common::get_pay_key_hash_with_attenuation_model_operations(
    const std::string& model_param, const receiver_record& record)
{
    if (model_param.empty()) {
        throw token_attenuation_model_exception("attenuation model param is empty.");
    }

    const wallet::payment_address payment(record.target);
    if (!payment) {
        throw toaddress_invalid_exception{"invalid target address"};
    }

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        return chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                payment.hash(), model_param, record.input_point);
    }

    throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
}

void base_transfer_common::populate_tx_outputs()
{
    for (const auto& iter: receiver_list_) {
        if (iter.is_empty()) {
            continue;
        }

        if (tx_item_idx_ >= (tx_limit + 10)) {
            throw tx_validate_exception{"Too many inputs/outputs,tx too large, canceled."};
        }
        tx_item_idx_++;

        auto&& payment_script = chain::script{ get_script_operations(iter) };

        // generate token info
        auto&& output_att = populate_output_asset(iter);
        set_uid_verify_asset(iter, output_att);

        if (!output_att.is_valid()) {
            throw tx_validate_exception{"validate transaction failure, invalid output asset."};
        }

        // fill output
        tx_.outputs.push_back({ iter.amount, payment_script, output_att });
    }
}

void base_transfer_common::populate_tx_inputs()
{
    // input args
    tx_input_type input;

    for (auto& fromeach : from_list_){
        if (tx_item_idx_ >= tx_limit) {
            auto&& response = "Too many inputs, suggest less than "
                + std::to_string(tx_limit) + " inputs.";
            throw tx_validate_exception(response);
        }

        tx_item_idx_++;
        input.sequence = max_input_sequence;
        input.previous_output.hash = fromeach.output.hash;
        input.previous_output.index = fromeach.output.index;
        tx_.inputs.push_back(input);
    }
}

void base_transfer_common::set_uid_verify_asset(const receiver_record& record, asset& attach)
{
    if (record.attach_elem.get_version() == UID_ASSET_VERIFY_VERSION) {
        attach.set_version(UID_ASSET_VERIFY_VERSION);
        attach.set_to_uid(record.attach_elem.get_to_uid());
        attach.set_from_uid(record.attach_elem.get_from_uid());
    }
}

asset base_transfer_common::populate_output_asset(const receiver_record& record)
{
    if ((record.type == utxo_attach_type::ucn)
        || (record.type == utxo_attach_type::deposit)
        || ((record.type == utxo_attach_type::token_transfer)
            && ((record.amount > 0) && (!record.token_amount)))) { // ucn
        return asset(UCN_TYPE, attach_version, chain::ucn(record.amount));
    }
    else if (record.type == utxo_attach_type::token_issue
        || record.type == utxo_attach_type::token_secondaryissue) {
        return asset(TOKEN_TYPE, attach_version, token(/*set on subclass*/));
    }
    else if (record.type == utxo_attach_type::token_transfer
            || record.type == utxo_attach_type::token_locked_transfer
            || record.type == utxo_attach_type::token_attenuation_transfer) {
        auto transfer = chain::token_transfer(record.symbol, record.token_amount);
        auto ass = token(TOKEN_TRANSFERABLE_TYPE, transfer);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid token transfer asset"};
        }
        return asset(TOKEN_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::message) {
        auto msg = boost::get<blockchain_message>(record.attach_elem.get_attach());
        if (msg.get_content().size() > 128) {
            throw tx_asset_value_exception{"memo text length should be less than 128"};
        }
        if (!msg.is_valid()) {
            throw tx_asset_value_exception{"invalid message asset"};
        }
        return asset(MESSAGE_TYPE, attach_version, msg);
    }
    else if (record.type == utxo_attach_type::uid_register) {
        uid_detail uiddetail(symbol_, record.target);
        auto ass = uid(UID_DETAIL_TYPE, uiddetail);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid uid register asset"};
        }
        return asset(UID_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::uid_transfer) {
        auto sh_uid = blockchain_.get_registered_uid(symbol_);
        if(!sh_uid)
            throw uid_symbol_notfound_exception{symbol_ + " not found"};

        sh_uid->set_address(record.target);
        auto ass = uid(UID_TRANSFERABLE_TYPE, *sh_uid);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid uid transfer asset"};
        }
        return asset(UID_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::token_cert
        || record.type == utxo_attach_type::token_cert_autoissue
        || record.type == utxo_attach_type::token_cert_issue
        || record.type == utxo_attach_type::token_cert_transfer) {
        if (record.token_cert == token_cert_ns::none) {
            throw token_cert_exception("token cert is none");
        }

        auto to_uid = record.attach_elem.get_to_uid();
        auto to_address = get_address_from_uid(to_uid, blockchain_);
        if (to_address != record.target) {
            throw token_cert_exception("address " + to_address + " dismatch uid " + to_uid);
        }

        auto cert_info = chain::token_cert(record.symbol, to_uid, to_address, record.token_cert);
        if (record.type == utxo_attach_type::token_cert_issue) {
            cert_info.set_status(TOKEN_CERT_ISSUE_TYPE);
        }
        else if (record.type == utxo_attach_type::token_cert_transfer) {
            cert_info.set_status(TOKEN_CERT_TRANSFER_TYPE);
        }
        else if (record.type == utxo_attach_type::token_cert_autoissue) {
            cert_info.set_status(TOKEN_CERT_AUTOISSUE_TYPE);
        }

        if (!cert_info.is_valid()) {
            throw tx_asset_value_exception{"invalid cert asset"};
        }
        return asset(TOKEN_CERT_TYPE, attach_version, cert_info);
    }
    else if (record.type == utxo_attach_type::token_card
        || record.type == utxo_attach_type::token_card_transfer) {
        return asset(TOKEN_CARD_TYPE, attach_version, token_card(/*set on subclass*/));
    }

    throw tx_asset_value_exception{
        "invalid utxo_attach_type value in receiver_record : "
            + std::to_string((uint32_t)record.type)};
}

bool base_transfer_common::filter_out_address(const std::string& address) const
{
    return blockchain_.is_script_address(address);
}

void base_transfer_helper::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    if(!from_.empty() && filter_out_address(from_))
    {
        throw tx_source_exception{"from address cannot be multi-signed. "};
    }

    // get from address balances
    for (auto& each : *pvaddr) {
        const auto& address = each.get_address();
        // filter script address
        if (filter_out_address(address)) {
            continue;
        }

        const auto priv_key = each.get_prv_key(passwd_);

        if (from_.empty()) {
            sync_fetchutxo(priv_key, address);
        } else if (from_ == address) {
            sync_fetchutxo(priv_key, address);
            // select ucn/token utxo only in from_ address
            check_payment_satisfied(FILTER_PAYFROM);
        } else {
            sync_fetchutxo(priv_key, address, FILTER_ALL_BUT_PAYFROM);
        }

        // performance improve
        if (is_payment_satisfied()) {
            break;
        }
    }

    //vote specify
    if (from_list_.empty() && symbol_ != UC_VOTE_TOKEN_SYMBOL) {
        throw tx_source_exception{"not enough ucn or token in from address"
            ", or you do not own the from address!(multisig address balance cannot be used in this way)"};
    }

    check_payment_satisfied();

    populate_change();
}

bool receiver_record::is_empty() const
{
    // has ucn amount
    if (amount != 0) {
        return false;
    }

    // ucn business , ucn == 0
    if ((type == utxo_attach_type::ucn) ||
        (type == utxo_attach_type::deposit)) {
        return true;
    }

    // has token amount
    if (token_amount != 0) {
        return false;
    }

    // token transfer business, ucn == 0 && token_amount == 0
    if ((type == utxo_attach_type::token_transfer) ||
        (type == utxo_attach_type::token_locked_transfer)) {
        return true;
    }

    // other business
    return false;
}

void base_transfer_common::check_tx()
{
    if (tx_.is_locktime_conflict()) {
        throw tx_locktime_exception{"The specified lock time is ineffective because all sequences"
            " are set to the maximum value."};
    }

    if (tx_.inputs.empty()) {
        throw tx_validate_exception{"validate transaction failure, empty inputs."};
    }

    if (tx_.outputs.empty()) {
        throw tx_validate_exception{"validate transaction failure, empty outputs."};
    }
}

std::string base_transfer_common::get_sign_tx_multisig_script(const address_token_record& from) const
{
    return "";
}

void base_transfer_common::sign_tx_inputs()
{
    uint32_t index = 0;
    for (auto& fromeach : from_list_)
    {
        bc::chain::script ss;

        // paramaters
        explorer::config::hashtype sign_type;
        uint8_t hash_type = (signature_hash_algorithm)sign_type;

        bc::explorer::config::ec_private config_private_key(fromeach.prikey);
        const ec_secret& private_key = config_private_key;

        std::string multisig_script = get_sign_tx_multisig_script(fromeach);
        if (!multisig_script.empty()) {
            bc::explorer::config::script config_contract(multisig_script);
            const bc::chain::script &contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                                                       contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"get_input_sign sign failure"};
            }

            // do script
            data_chunk data;
            ss.operations.push_back({bc::chain::opcode::zero, data});
            ss.operations.push_back({bc::chain::opcode::special, endorse});

            chain::script script_encoded;
            script_encoded.from_string(multisig_script);

            ss.operations.push_back({bc::chain::opcode::pushdata1, script_encoded.to_data(false)});
        }
        else {
            bc::explorer::config::script config_contract(fromeach.script);
            const bc::chain::script& contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"get_input_sign sign failure"};
            }

            // do script
            bc::wallet::ec_private ec_private_key(private_key, 0u, true);
            auto&& public_key = ec_private_key.to_public();
            data_chunk public_key_data;
            public_key.to_data(public_key_data);

            ss.operations.push_back({bc::chain::opcode::special, endorse});
            ss.operations.push_back({bc::chain::opcode::special, public_key_data});

            // if pre-output script is deposit tx.
            if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
                uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                    contract.operations);
                ss.operations.push_back({bc::chain::opcode::special, script_number(lock_height).data()});
            }
        }

        // set input script of this tx
        tx_.inputs[index].script = ss;
        index++;
    }
}

void base_transfer_common::send_tx()
{
    if(blockchain_.validate_transaction(tx_)) {
#ifdef UC_DEBUG
        throw tx_validate_exception{"validate transaction failure. " + tx_.to_string(1)};
#endif
        throw tx_validate_exception{"validate transaction failure"};
    }
    if(blockchain_.broadcast_transaction(tx_))
        throw tx_broadcast_exception{"broadcast transaction failure"};
}

void base_transfer_common::populate_tx_header()
{
    tx_.locktime = 0;
    if (validate_transaction::is_nova_feature_activated(blockchain_)) {
        tx_.version = transaction_version::check_nova_feature;
    } else {
        tx_.version = transaction_version::check_output_script;
    }
}

void base_transfer_common::exec()
{
    // prepare
    sum_payment_amount();
    populate_unspent_list();

    // construct tx
    populate_tx_header();
    populate_tx_inputs();
    populate_tx_outputs();

    // check tx
    check_tx();

    // sign tx
    sign_tx_inputs();

    // send tx
    send_tx();
}

void base_multisig_transfer_helper::send_tx()
{
    auto from_address = multisig_.get_address();
    if (from_address.empty()) {
        base_transfer_common::send_tx();
    }
    else {
        // no operation in exec for transferring multisig token cert
    }
}

bool base_multisig_transfer_helper::filter_out_address(const std::string& address) const
{
    auto multisig_address = multisig_.get_address();
    if (multisig_address.empty()) {
        return base_transfer_common::filter_out_address(address);
    }
    else {
        return address != multisig_address;
    }
}

std::string base_multisig_transfer_helper::get_sign_tx_multisig_script(const address_token_record& from) const
{
    return multisig_.get_multisig_script();
}

void base_transaction_constructor::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (from_vec_.empty()) {
        throw fromaddress_empty_exception{"empty from address"};
    }
}

void base_transaction_constructor::populate_change()
{
    // ucn utxo
    populate_ucn_change();

    // token utxo
    populate_token_change();

    if (!message_.empty()) { // ucn transfer/token transfer  -- with message
        auto addr = !mychange_.empty() ? mychange_ : from_list_.begin()->addr;
        receiver_list_.push_back({addr, "", 0, 0,
            utxo_attach_type::message,
            asset(0, 0, blockchain_message(message_))});
    }
}

void base_transaction_constructor::populate_unspent_list()
{
    // get from address balances
    for (auto& each : from_vec_) {
        sync_fetchutxo("", each);
        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough ucn or token in the from address!"};
    }

    check_payment_satisfied();

    // change
    populate_change();
}

const std::vector<uint16_t> depositing_ucn::vec_cycle{10, 45, 120, 240, 540};

uint32_t depositing_ucn::get_reward_lock_height() const
{
    int index = 0;
    auto it = std::find(vec_cycle.begin(), vec_cycle.end(), deposit_cycle_);
    if (it != vec_cycle.end()) { // found cycle
        index = std::distance(vec_cycle.begin(), it);
    }

    return (uint32_t)bc::consensus::lock_heights[index];
}

chain::operation::stack
depositing_ucn::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and token should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        const auto& hash = payment.hash();
        if((to_ == record.target)
            && (utxo_attach_type::deposit == record.type)) {
            payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_reward_lock_height());
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
        }
    }
    else {
        throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
    }

    return payment_ops;
}

const std::vector<uint16_t> depositing_ucn_transaction::vec_cycle{10, 45, 120, 240, 540};

uint32_t depositing_ucn_transaction::get_reward_lock_height() const
{
    int index = 0;
    auto it = std::find(vec_cycle.begin(), vec_cycle.end(), deposit_);
    if (it != vec_cycle.end()) { // found cycle
        index = std::distance(vec_cycle.begin(), it);
    }

    return (uint32_t)bc::consensus::lock_heights[index];
}

chain::operation::stack
depositing_ucn_transaction::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and token should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        const auto& hash = payment.hash();
        if((utxo_attach_type::deposit == record.type)) {
            payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(
                hash, get_reward_lock_height());
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
        }
    }
    else {
        throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
    }

    return payment_ops;
}

uint32_t voting_token::get_reward_lock_height() const
{
    return 48*60*60*2;//48h
}

chain::operation::stack
voting_token::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and token should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        const auto& hash = payment.hash();
        if((to_ == record.target)
            && (utxo_attach_type::deposit == record.type)) {
            payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_reward_lock_height());
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
        }
    }
    else {
        throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
    }

    return payment_ops;
}

void sending_multisig_tx::populate_change()
{
    // ucn utxo
    populate_ucn_change();

    // token utxo
    populate_token_change();
}

void issuing_token::sum_payments()
{
    for (auto& iter : receiver_list_) {
        payment_ucn_ += iter.amount;
        payment_token_ += iter.token_amount;

        if (iter.token_cert == token_cert_ns::domain) {
            auto&& domain = token_cert::get_domain(symbol_);
            if (!token_cert::is_valid_domain(domain)) {
                throw token_cert_domain_exception{"no valid domain exists for token : " + symbol_};
            }
            if (blockchain_.is_token_cert_exist(domain, token_cert_ns::domain)) {
                payment_token_cert_.clear();
                payment_token_cert_.push_back(token_cert_ns::domain); // will verify by input
            }
        }
        else if (iter.token_cert == token_cert_ns::naming) {
            auto&& domain = token_cert::get_domain(symbol_);
            if (!token_cert::is_valid_domain(domain)) {
                throw token_cert_domain_exception{"no valid domain exists for token : " + symbol_};
            }

            if (blockchain_.is_token_cert_exist(symbol_, token_cert_ns::naming)) {
                payment_token_cert_.clear();
                payment_token_cert_.push_back(token_cert_ns::naming); // will verify by input
            }
            else {
                throw token_cert_notfound_exception{"no naming cert exists for token : " + symbol_};
            }
        }
    }
}

void issuing_token::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    unissued_token_ = blockchain_.get_account_unissued_token(name_, symbol_);
    if (!unissued_token_) {
        throw token_symbol_notfound_exception{symbol_ + " not created"};
    }

    uint64_t min_fee = bc::min_fee_to_issue_token;
    if (payment_ucn_ < min_fee) {
        throw token_issue_poundage_exception("fee must at least "
            + std::to_string(min_fee) + " satoshi == "
            + std::to_string(min_fee/100000000) + " ucn");
    }

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, unissued_token_->get_maximum_supply());
    }

    uint64_t amount = (uint64_t)std::floor(payment_ucn_ * ((100 - fee_percentage_to_miner_) / 100.0));
    if (amount > 0) {
        auto&& address = bc::get_developer_community_address(blockchain_.chain_settings().use_testnet_rules);
        auto&& uid = blockchain_.get_uid_from_address(address);
        receiver_list_.push_back({address, "", amount, 0, utxo_attach_type::ucn, asset("", uid)});
    }
}

chain::operation::stack
issuing_token::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::token_issue == record.type)) {
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

asset issuing_token::populate_output_asset(const receiver_record& record)
{
    asset&& attach = base_transfer_common::populate_output_asset(record);

    if (record.type == utxo_attach_type::token_issue) {
        unissued_token_->set_address(record.target);
        auto ass = token(TOKEN_DETAIL_TYPE, *unissued_token_);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid token issue asset"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

void sending_token::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, payment_token_);
    }
}

void sending_token::populate_change()
{
    // ucn utxo
    populate_ucn_change();

    // token utxo
    populate_token_change();

    // token transfer  -- with message
    if (!message_.empty()) {
        auto addr = !mychange_.empty() ? mychange_ : from_list_.begin()->addr;
        receiver_list_.push_back({addr, "", 0, 0,
            utxo_attach_type::message,
            asset(0, 0, blockchain_message(message_))});
    }
}

chain::operation::stack
sending_token::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::token_attenuation_transfer == record.type)) { // for sending token only
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

chain::operation::stack
secondary_issuing_token::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::token_secondaryissue == record.type)) {
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

void secondary_issuing_token::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    target_address_ = receiver_list_.begin()->target;

    issued_token_ = blockchain_.get_issued_token(symbol_);
    if (!issued_token_) {
        throw token_symbol_notfound_exception{"token symbol does not exist on the blockchain"};
    }

    auto total_volume = blockchain_.get_token_volume(symbol_);
    if (total_volume > max_uint64 - volume_) {
        throw token_amount_exception{"secondaryissue, volume cannot exceed maximum value"};
    }

    if (!token_cert::test_certs(payment_token_cert_, token_cert_ns::issue)) {
        throw token_cert_exception("no token cert of issue right is provided.");
    }

    if (blockchain_.chain_settings().use_testnet_rules
        && !blockchain_.is_token_cert_exist(symbol_, token_cert_ns::issue)) {
        payment_token_cert_.clear();
    }

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, volume_);
    }
}

void secondary_issuing_token::populate_change()
{
    // ucn utxo
    populate_ucn_change();

    // token utxo
    if (payment_token_ > 0) {
        receiver_list_.push_back({target_address_, symbol_,
            0, payment_token_,
            utxo_attach_type::token_transfer, asset()});
    }
    populate_token_change(target_address_);
}

asset secondary_issuing_token::populate_output_asset(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_asset(record);

    if (record.type == utxo_attach_type::token_secondaryissue) {
        auto token_detail = *issued_token_;
        token_detail.set_address(record.target);
        token_detail.set_token_secondaryissue();
        token_detail.set_maximum_supply(volume_);
        token_detail.set_issuer(record.attach_elem.get_to_uid());
        auto ass = token(TOKEN_DETAIL_TYPE, token_detail);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid token secondary issue asset"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

void issuing_token_cert::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    if (token_cert::test_certs(payment_token_cert_, token_cert_ns::naming)) {
        if (!token_cert::test_certs(payment_token_cert_, token_cert_ns::domain)) {
            throw token_cert_exception("no token cert of domain right.");
        }

        payment_token_cert_.clear();
        payment_token_cert_.push_back(token_cert_ns::domain);
    }
    else {
        payment_token_cert_.clear();
    }
}

void registering_uid::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    uint64_t min_fee = bc::min_fee_to_register_uid;
    if (payment_ucn_ < min_fee) {
        throw uid_register_poundage_exception("fee must at least "
            + std::to_string(min_fee) + " satoshi == "
            + std::to_string(min_fee/100000000) + " ucn");
    }

    uint64_t amount = (uint64_t)std::floor(payment_ucn_ * ((100 - fee_percentage_to_miner_) / 100.0));
    if (amount > 0) {
        auto&& address = bc::get_developer_community_address(blockchain_.chain_settings().use_testnet_rules);
        auto&& uid = blockchain_.get_uid_from_address(address);
        receiver_list_.push_back({address, "", amount, 0, utxo_attach_type::ucn, asset("", uid)});
    }
}

std::string sending_multisig_uid::get_sign_tx_multisig_script(const address_token_record& from) const
{
    std::string multisig_script;
    if (from.addr == multisig_from_.get_address()) {
        multisig_script = multisig_from_.get_multisig_script();

    }
    else if (from.addr == multisig_to_.get_address()) {
        multisig_script = multisig_to_.get_multisig_script();
    }
    return multisig_script;
}

void sending_multisig_uid::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (fromfee.empty()) {
        throw fromaddress_empty_exception{"empty fromfee address"};
    }
}

void sending_multisig_uid::populate_change()
{
    // ucn utxo
    populate_ucn_change(fromfee);
}

void sending_multisig_uid::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    // get from address balances
    for (auto& each : *pvaddr) {

        if (fromfee == each.get_address()) {
            // pay fee
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_UCN);
            check_payment_satisfied(FILTER_UCN);
        }

        if (from_ == each.get_address()) {
            // pay uid
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_UID);
            check_payment_satisfied(FILTER_UID);
        }

        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough ucn or token in from address"
            ", or you do not own the from address!"};
    }

    check_payment_satisfied();

    populate_change();
}

void sending_uid::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (fromfee.empty()) {
        throw fromaddress_empty_exception{"empty fromfee address"};
    }
}

void sending_uid::populate_change()
{
    // ucn utxo
    populate_ucn_change(fromfee);
}

void sending_uid::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    // get from address balances
    for (auto& each : *pvaddr) {
        // filter script address
        if (blockchain_.is_script_address(each.get_address()))
            continue;

        if (fromfee == each.get_address()) {
            // pay fee
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_UCN);
            check_payment_satisfied(FILTER_UCN);
        }

        if (from_ == each.get_address()) {
            // pay uid
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_UID);
            check_payment_satisfied(FILTER_UID);
        }

        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough ucn or token in from address"
            ", or you do not own the from address!"};
    }

    check_payment_satisfied();

    populate_change();
}

asset registering_card::populate_output_asset(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_asset(record);

    if (record.type == utxo_attach_type::token_card) {
        auto iter = card_map_.find(record.symbol);
        if (iter == card_map_.end()) {
            throw tx_asset_value_exception{"invalid card issue asset"};
        }

        auto ass = token_card(record.symbol, record.target, iter->second);
        ass.set_status(CARD_STATUS_REGISTER);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid card issue asset"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

asset transferring_card::populate_output_asset(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_asset(record);

    if (record.type == utxo_attach_type::token_card_transfer) {
        auto ass = token_card(record.symbol, record.target, "");
        ass.set_status(CARD_STATUS_TRANSFER);
        if (!ass.is_valid()) {
            throw tx_asset_value_exception{"invalid card transfer asset"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

} //commands
} // explorer
} // libbitcoin
