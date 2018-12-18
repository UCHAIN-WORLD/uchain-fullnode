/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-api.
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
#pragma once

#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/command.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChainService/consensus/miner.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands{

/// NOTICE: this type is not equal to asset_type and business_kind
/// asset_type : the collapsed type of tx output asset, **recorded on blockchain**
/// business_kind   : the expanded type of asset, mainly used for database/history query
/// for example :
/// asset_type           |  business_kind
/// -------------------------------------------------------------------
/// asset_ucn           --> ucn
/// asset_ucn_award     --> ucn_award
/// asset_token         --> token_issue | token_transfer
/// asset_message       --> message
/// asset_uid           --> uid_register   |  uid_transfer
/// asset_token_cert    --> token_cert
/// asset_token_candidate     --> token_candidate
/// -------------------------------------------------------------------
/// utxo_attach_type is only used in explorer module
/// utxo_attach_type will be used to generate asset with asset_type and content
/// for example :
/// utxo_attach_type::token_issue    --> asset_token of token_detail
///     auto token_detail = token(TOKEN_DETAIL_TYPE, token_detail);
///     asset(UC_TOKEN_TYPE, attach_version, token_detail);
/// utxo_attach_type::token_transfer --> asset_token of token_transfer
///     auto token_transfer = token(TOKEN_TRANSFERABLE_TYPE, token_transfer);
///     asset(UC_TOKEN_TYPE, attach_version, token_transfer);
/// NOTICE: createrawtx / createmultisigtx --type option is using these values.
/// DO NOT CHANGE EXIST ITEMS!!!
enum class utxo_attach_type : uint32_t
{
    ucn = 0,
    deposit = 1,
    token_issue = 2,
    token_transfer = 3,
    token_attenuation_transfer = 4,
    token_locked_transfer = 5,
    message = 6,
    token_cert = 7,
    token_secondaryissue = 8,
    uid_register = 9,
    uid_transfer = 10,
    token_cert_issue = 11,
    token_cert_transfer = 12,
    token_cert_autoissue = 13,
    token_candidate = 14,
    token_candidate_transfer = 15,
    invalid = 0xffffffff
};

extern utxo_attach_type get_utxo_attach_type(const chain::output&);

struct address_token_record
{
    std::string prikey;
    std::string addr;
    uint64_t    amount{0}; // spendable ucn amount
    std::string symbol;
    uint64_t    token_amount{0}; // spendable token amount
    token_cert_type token_cert{token_cert_ns::none};
    utxo_attach_type type{utxo_attach_type::invalid};
    output_point output;
    chain::script script;
    uint32_t hd_index{0}; // only used for multisig tx
};

struct receiver_record
{
    typedef std::vector<receiver_record> list;

    std::string target;
    std::string symbol;
    uint64_t    amount{0}; // ucn value
    uint64_t    token_amount{0};
    token_cert_type token_cert{token_cert_ns::none};

    utxo_attach_type type{utxo_attach_type::invalid};
    asset attach_elem;  // used for MESSAGE_TYPE, used for information transfer etc.
    chain::input_point input_point{null_hash, max_uint32};

    receiver_record()
        : target()
        , symbol()
        , amount(0)
        , token_amount(0)
        , token_cert(token_cert_ns::none)
        , type(utxo_attach_type::invalid)
        , attach_elem()
        , input_point{null_hash, max_uint32}
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t token_amount_,
        utxo_attach_type type_, const asset& attach_elem_ = asset(),
        const chain::input_point& input_point_ = {null_hash, max_uint32})
        : receiver_record(target_, symbol_, amount_, token_amount_,
            token_cert_ns::none, type_, attach_elem_, input_point_)
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t token_amount_, token_cert_type token_cert_,
        utxo_attach_type type_, const asset& attach_elem_ = asset(),
        const chain::input_point& input_point_ = {null_hash, max_uint32})
        : target(target_)
        , symbol(symbol_)
        , amount(amount_)
        , token_amount(token_amount_)
        , token_cert(token_cert_)
        , type(type_)
        , attach_elem(attach_elem_)
        , input_point(input_point_)
    {}

    bool is_empty() const;
};

struct balances {
    uint64_t total_received;
    uint64_t confirmed_balance;
    uint64_t unspent_balance;
    uint64_t frozen_balance;
};

struct deposited_balance {
    deposited_balance(const std::string& address_, const string& tx_hash_,
        uint64_t deposited_, uint64_t expiration_)
        : address(address_)
        , tx_hash(tx_hash_)
        , balance(0)
        , bonus(0)
        , deposited_height(deposited_)
        , expiration_height(expiration_)
    {}

    std::string address;
    std::string tx_hash;
    std::string bonus_hash;
    uint64_t balance;
    uint64_t bonus;
    uint64_t deposited_height;
    uint64_t expiration_height;

    // for sort
    bool operator< (const deposited_balance& other) const {
        return expiration_height < other.expiration_height;
    }

    typedef std::vector<deposited_balance> list;
};

// helper function
void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance);

void sync_fetch_deposited_balance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<deposited_balance::list> sh_vec);

void sync_fetch_token_balance(const std::string& address, bool sum_all,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_balances::list> sh_token_vec);

void sync_fetch_token_deposited_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_deposited_balance::list> sh_token_vec);

std::shared_ptr<token_balances::list> sync_fetch_token_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain);

std::shared_ptr<token_deposited_balance::list> sync_fetch_token_deposited_view(
    const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain);
    

void sync_fetch_token_cert_balance(const std::string& address, const string& symbol,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<token_cert::list> sh_vec, token_cert_type cert_type=token_cert_ns::none);

std::string get_random_payment_address(std::shared_ptr<std::vector<account_address>>,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address(const std::string& uid_or_address,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address(const std::string& uid_or_address,
    asset& attach, bool is_from,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address_from_uid(const std::string& uid,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_fee_dividend_address(bc::blockchain::block_chain_impl& blockchain);

void check_token_symbol(const std::string& symbol, bool check_sensitive=false);
void check_candidate_symbol(const std::string& symbol, bool check_sensitive=false);
void check_uid_symbol(const std::string& symbol,  bool check_sensitive=false);
void check_token_symbol_with_miner(const std::string& symbol, const consensus::miner& miner, const std::string& address);
void check_token_symbol_with_method(const std::string& symbol);

class BCX_API base_transfer_common
{
public:
    enum filter : uint8_t {
        FILTER_UCN = 1 << 0,
        FILTER_TOKEN = 1 << 1,
        FILTER_TOKENCERT = 1 << 2,
        FILTER_UID = 1 << 3,
        FILTER_IDENTIFIABLE_TOKEN = 1 << 4,
        FILTER_ALL = 0xff,
        // if specify 'from_' address,
        // then get these types' unspent only from 'from_' address
        FILTER_PAYFROM = FILTER_UCN | FILTER_TOKEN,
        FILTER_ALL_BUT_PAYFROM = FILTER_ALL & ~FILTER_PAYFROM
    };

    base_transfer_common(
        bc::blockchain::block_chain_impl& blockchain,
        receiver_record::list&& receiver_list, uint64_t fee,
        std::string&& symbol, std::string&& from, std::string&& change)
        : blockchain_{blockchain}
        , symbol_{std::move(symbol)}
        , from_{std::move(from)}
        , mychange_{std::move(change)}
        , payment_ucn_{fee}
        , receiver_list_{std::move(receiver_list)}
    {
    };

    virtual ~base_transfer_common()
    {
        receiver_list_.clear();
        from_list_.clear();
    };

    static const uint64_t maximum_fee{10000000000};
    static const uint64_t minimum_fee{10000};
    static const uint64_t tx_limit{677};
    static const uint64_t attach_version{1};

    virtual bool get_spendable_output(chain::output&, const chain::history&, uint64_t height) const;
    virtual chain::operation::stack get_script_operations(const receiver_record& record) const;
    virtual void sync_fetchutxo(
            const std::string& prikey, const std::string& addr, filter filter = FILTER_ALL);
    virtual asset populate_output_asset(const receiver_record& record);
    virtual void sum_payments();
    virtual void sum_payment_amount();
    virtual void populate_change();
    virtual void populate_tx_outputs();
    virtual void populate_unspent_list() = 0;
    virtual void sign_tx_inputs();
    virtual void send_tx();
    virtual void populate_tx_header();

    // common functions, single responsibility.
    static void check_fee_in_valid_range(uint64_t fee);
    void check_receiver_list_not_empty() const;
    bool is_payment_satisfied(filter filter = FILTER_ALL) const;
    void check_payment_satisfied(filter filter = FILTER_ALL) const;
    void check_model_param_initial(std::string& param, uint64_t amount);

    static chain::operation::stack get_pay_key_hash_with_attenuation_model_operations(
            const std::string& model_param, const receiver_record& record);

    void populate_ucn_change(const std::string& address = std::string(""));
    void populate_token_change(const std::string& address = std::string(""));
    void populate_tx_inputs();
    void check_tx();
    void exec();

    std::string get_mychange_address(filter filter) const;

    tx_type& get_transaction() { return tx_; }
    const tx_type& get_transaction() const { return tx_; }

    // in secondary issue, locked token can also verify threshold condition
    virtual bool is_locked_token_as_payment() const {return false;}

    virtual bool filter_out_address(const std::string& address) const;

    virtual std::string get_sign_tx_multisig_script(const address_token_record& from) const;

    void set_uid_verify_asset(const receiver_record& record, asset& attach);

protected:
    bc::blockchain::block_chain_impl& blockchain_;
    tx_type                           tx_; // target transaction
    std::string                       symbol_;
    std::string                       from_;
    std::string                       mychange_;
    uint64_t                          tx_item_idx_{0};
    uint64_t                          payment_ucn_{0};
    uint64_t                          payment_token_{0};
    uint64_t                          unspent_ucn_{0};
    uint64_t                          unspent_token_{0};
    std::vector<token_cert_type>      payment_token_cert_;
    std::vector<token_cert_type>      unspent_token_cert_;
    uint8_t                           payment_uid_{0};
    uint8_t                           unspent_uid_{0};
    uint8_t                           payment_candidate_{0};
    uint8_t                           unspent_candidate_{0};
    std::vector<receiver_record>      receiver_list_;
    std::vector<address_token_record> from_list_;
};

class BCX_API base_transfer_helper : public base_transfer_common
{
public:
    base_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee, std::string&& symbol = std::string(""),
        std::string&& change = std::string(""))
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), std::move(from),
            std::move(change))
        , cmd_{cmd}
        , name_{std::move(name)}
        , passwd_{std::move(passwd)}
    {}

    ~base_transfer_helper()
    {}

    void populate_unspent_list() override;

protected:
    command&                          cmd_;
    std::string                       name_;
    std::string                       passwd_;
};

class BCX_API base_multisig_transfer_helper : public base_transfer_helper
{
public:
    base_multisig_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee, std::string&& symbol,
        account_multisig&& multisig_from)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , multisig_(std::move(multisig_from))
    {}

    ~base_multisig_transfer_helper()
    {}

    bool filter_out_address(const std::string& address) const override;

    std::string get_sign_tx_multisig_script(const address_token_record& from) const override;

    void send_tx() override;

protected:
    // for multisig address
    account_multisig multisig_;
};

class BCX_API base_transaction_constructor : public base_transfer_common
{
public:
    base_transaction_constructor(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, receiver_record::list&& receiver_list,
        std::string&& symbol, std::string&& change,
        std::string&& message, uint64_t fee)
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), "", std::move(change))
        , type_{type}
        , message_{std::move(message)}
        , from_vec_{std::move(from_vec)}
    {}

    virtual ~base_transaction_constructor()
    {
        from_vec_.clear();
    };

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    // no operation in exec
    void sign_tx_inputs() override {}
    void send_tx() override {}

protected:
    utxo_attach_type                  type_{utxo_attach_type::invalid};
    std::string                       message_;
    std::vector<std::string>          from_vec_; // from address vector
};

class BCX_API depositing_ucn : public base_transfer_helper
{
public:
    depositing_ucn(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& to, receiver_record::list&& receiver_list,
        uint16_t deposit_cycle = 7, uint64_t fee = 10000)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::string(""), std::move(receiver_list), fee)
        , to_{std::move(to)}
        , deposit_cycle_{deposit_cycle}
    {}

    ~depositing_ucn(){}

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height() const;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::string                       to_;
    uint16_t                          deposit_cycle_{7}; // 7 days
};

class BCX_API depositing_ucn_transaction : public base_transaction_constructor
{
public:
    depositing_ucn_transaction(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, receiver_record::list&& receiver_list,
        uint16_t deposit, std::string&& change,
        std::string&& message, uint64_t fee)
        : base_transaction_constructor(blockchain, type, std::forward<std::vector<std::string>>(from_vec),
            std::move(receiver_list), std::string(""),
            std::move(change), std::move(message), fee)
        , deposit_{deposit}
    {}

    ~depositing_ucn_transaction(){}

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height() const;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    uint16_t                          deposit_{7}; // 7 days
};

class BCX_API voting_token : public base_transfer_helper
{
public:
    voting_token(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& to, receiver_record::list&& receiver_list,
        uint16_t amount , uint64_t fee = 10000)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            "", std::move(receiver_list), fee ,std::string(UC_VOTE_TOKEN_SYMBOL))
        , to_{std::move(to)}
        , amount_{amount}
    {}

    ~voting_token(){}

    uint32_t get_reward_lock_height() const;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::string                       to_;
    uint64_t                          amount_; 
};

class BCX_API sending_ucn : public base_transfer_helper
{
public:
    sending_ucn(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        std::string&& change, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, "", std::move(change))
    {}

    ~sending_ucn(){}
};

class BCX_API sending_multisig_tx : public base_multisig_transfer_helper
{
public:
    sending_multisig_tx(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig& multisig, std::string&& symbol = std::string(""))
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig))
    {}

    ~sending_multisig_tx(){}

    void populate_change() override;
};

class BCX_API issuing_token : public base_transfer_helper
{
public:
    issuing_token(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee, uint32_t fee_percentage_to_miner)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , attenuation_model_param_{std::move(model_param)}
        , fee_percentage_to_miner_(fee_percentage_to_miner)
    {}

    ~issuing_token(){}

    void sum_payments() override;
    void sum_payment_amount() override;
    asset populate_output_asset(const receiver_record& record) override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::shared_ptr<token_detail> unissued_token_;
    std::string domain_cert_address_;
    std::string attenuation_model_param_;
    uint32_t fee_percentage_to_miner_;
};

class BCX_API secondary_issuing_token : public base_transfer_helper
{
public:
    secondary_issuing_token(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee, uint64_t volume)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , volume_(volume)
        , attenuation_model_param_{std::move(model_param)}
    {}

    ~secondary_issuing_token(){}

    void sum_payment_amount() override;
    void populate_change() override;
    asset populate_output_asset(const receiver_record& record) override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

    uint64_t get_volume() { return volume_; };

    bool is_locked_token_as_payment() const override {return true;}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

private:
    uint64_t volume_{0};
    std::shared_ptr<token_detail> issued_token_;
    std::string target_address_;
    std::string attenuation_model_param_;
};

class BCX_API sending_token : public base_transfer_helper
{
public:
    sending_token(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee,
        std::string&& message, std::string&& change)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee,
            std::move(symbol), std::move(change))
        , attenuation_model_param_{std::move(model_param)}
        , message_{std::move(message)}
    {}

    ~sending_token()
    {}

    void sum_payment_amount() override;
    void populate_change() override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::string attenuation_model_param_;
    std::string message_;
};

class BCX_API registering_uid : public base_multisig_transfer_helper
{
public:
    registering_uid(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, receiver_record::list&& receiver_list,
        uint64_t fee, uint32_t fee_percentage_to_miner,
        account_multisig&& multisig)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig))
        , fee_percentage_to_miner_(fee_percentage_to_miner)
    {}

    ~registering_uid()
    {}

    void sum_payment_amount() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

private:
    uint32_t fee_percentage_to_miner_;
};

class BCX_API sending_multisig_uid : public base_transfer_helper
{
public:
    sending_multisig_uid(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list
        , uint64_t fee, account_multisig&& multisig, account_multisig&& multisigto)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , fromfee(feefrom), multisig_from_(std::move(multisig)), multisig_to_(std::move(multisigto))
    {}

    ~sending_multisig_uid()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    std::string get_sign_tx_multisig_script(const address_token_record& from) const override;

    // no operation in exec
    void send_tx() override {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

private:
    std::string fromfee;
    account_multisig multisig_from_;
    account_multisig multisig_to_;
};

class BCX_API sending_uid : public base_transfer_helper
{
public:
    sending_uid(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol)),fromfee(feefrom)
    {}

    ~sending_uid()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

private:
    std::string fromfee;
};

class BCX_API transferring_token_cert : public base_multisig_transfer_helper
{
public:
    transferring_token_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig&& multisig_from)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig_from))
    {}

    ~transferring_token_cert()
    {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };
};

class BCX_API issuing_token_cert : public base_transfer_helper
{
public:
    issuing_token_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
    {}

    ~issuing_token_cert()
    {}

    void sum_payment_amount() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };
};
class BCX_API registering_candidate : public base_transfer_helper
{
public:
    registering_candidate(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, std::map<std::string, std::string>&& candidate_map,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , candidate_map_(candidate_map)
    {}

    ~registering_candidate()
    {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

    asset populate_output_asset(const receiver_record& record) override;

private:
    std::map<std::string, std::string> candidate_map_;
};

class BCX_API transferring_candidate : public base_multisig_transfer_helper
{
public:
    transferring_candidate(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig&& multisig_from)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig_from))
    {}

    ~transferring_candidate()
    {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_uid_feature;
        tx_.locktime = 0;
    };

    asset populate_output_asset(const receiver_record& record) override;
};


} // commands
} // explorer
} // libbitcoin
