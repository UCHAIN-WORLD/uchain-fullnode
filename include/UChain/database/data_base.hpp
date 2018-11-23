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
#ifndef UC_DATABASE_DATA_BASE_HPP
#define UC_DATABASE_DATA_BASE_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/databases/block_database.hpp>
#include <UChain/database/databases/spend_database.hpp>
#include <UChain/database/databases/transaction_database.hpp>
#include <UChain/database/databases/history_database.hpp>
#include <UChain/database/databases/stealth_database.hpp>
#include <UChain/database/define.hpp>
#include <UChain/database/settings.hpp>

#include <boost/variant.hpp>
#include <UChainService/txs/token/token.hpp>
#include <UChainService/txs/ucn/ucn.hpp>
#include <UChainService/txs/message/message.hpp>
#include <UChainService/txs/account/account.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <UChainService/txs/token/token_transfer.hpp>
#include <UChainService/txs/asset.hpp>

#include <UChainService/data/databases/account_database.hpp>
#include <UChainService/data/databases/account_address_database.hpp>
#include <UChainService/data/databases/token_database.hpp>
#include <UChainService/data/databases/blockchain_token_database.hpp>
#include <UChainService/data/databases/address_token_database.hpp>
#include <UChainService/data/databases/account_token_database.hpp>
#include <UChainService/txs/uid/uid.hpp>
#include <UChainService/data/databases/blockchain_token_cert_database.hpp>
#include <UChainService/data/databases/blockchain_uid_database.hpp>
#include <UChainService/data/databases/address_uid_database.hpp>
#include <UChainService/data/databases/blockchain_card_database.hpp>
#include <UChainService/data/databases/address_card_database.hpp>
#include <UChainService/data/databases/card_history_database.hpp>

using namespace libbitcoin::wallet;
using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

typedef uint64_t handle;

class BCD_API data_base
{
public:
    typedef boost::filesystem::path path;

    class store
    {
    public:
        store(const path& prefix);
        bool touch_all() const;
        bool touch_uids() const;
        bool uids_exist() const;
        bool touch_tokens() const;
        bool tokens_exist() const;
        bool touch_certs() const;
        bool certs_exist() const;
        bool touch_cards() const;
        bool mits_exist() const;

        path database_lock;
        path blocks_lookup;
        path blocks_index;
        path history_lookup;
        path history_rows;
        path stealth_rows;
        path spends_lookup;
        path transactions_lookup;
        /* begin database for account, token, address_token, uid relationship */
        path accounts_lookup;
        path tokens_lookup;
        path certs_lookup;
        path address_tokens_lookup;
        path address_tokens_rows;
        path account_tokens_lookup;
        path account_tokens_rows;
        path uids_lookup;
        path address_uids_lookup;
        path address_uids_rows;
        path account_addresses_lookup;
        path account_addresses_rows;
        /* end database for account, token, address_token, uid ,address_uid relationship */
        path mits_lookup;
        path address_cards_lookup;
        path address_cards_rows;
        path card_history_lookup;
        path card_history_rows;
    };

    class db_metadata
    {
    public:
        db_metadata();
        db_metadata(std::string version);
        void reset();
        bool from_data(const data_chunk& data);
        bool from_data(std::istream& stream);
        bool from_data(reader& source);
        data_chunk to_data() const;
        void to_data(std::ostream& stream) const;
        void to_data(writer& sink) const;
        uint64_t serialized_size() const;

#ifdef UC_DEBUG
        std::string to_string() const;
#endif
        friend std::istream& operator>>(std::istream& input, db_metadata& metadata);
        friend std::ostream& operator<<(std::ostream& output, const db_metadata& metadata);
        static const std::string current_version;
        static const std::string file_name;

        std::string version_;
    };

    /// Create a new database file with a given path prefix and default paths.
    static bool initialize(const path& prefix, const chain::block& genesis);
    /// If database exists then upgrades to version 63.
    static bool upgrade_version_63(const path& prefix);

    static bool touch_file(const path& file_path);
    static void write_metadata(const path& metadata_path, data_base::db_metadata& metadata);
    static void read_metadata(const path& metadata_path, data_base::db_metadata& metadata);
    /// Construct all databases.
    data_base(const settings& settings);

    /// Stop all databases (threads must be joined).
    ~data_base();
    // Startup and shutdown.
    // ------------------------------------------------------------------------

    /// Create and start all databases.
    bool create();
    bool create_uids();
    bool create_tokens();
    bool create_certs();
    bool create_cards();

    /// Start all databases.
    bool start();

    /// Signal all databases to stop work.
    bool stop();

    /// Stop all databases (threads must be joined).
    bool close();

    // Locking.
    // ------------------------------------------------------------------------

    handle begin_read();
    bool begin_write();
    bool end_write();
    bool is_read_valid(handle handle);
    bool is_write_locked(handle handle);

    // Push and pop.
    // ------------------------------------------------------------------------

    /// Commit block at next height with indexing and no duplicate protection.
    void push(const chain::block& block);

    /// Commit block at given height with indexing and no duplicate protection.
    /// If height is not count + 1 then the count will not equal top height.
    void push(const chain::block& block, uint64_t height);

    /// Throws if the chain is empty.
    chain::block pop();

    /* begin store token info into  database */

    void push_asset(const asset& attach, const payment_address& address,
            const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_ucn(const ucn& ucn, const short_hash& key,
            const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_ucn_award(const ucn_award& ucn, const short_hash& key,
            const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_message(const chain::blockchain_message& msg, const short_hash& key,
            const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_token(const token& sp, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_token_cert(const token_cert& sp_cert, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_token_detail(const token_detail& sp_detail, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_token_transfer(const token_transfer& sp_transfer, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_uid(const uid& sp, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_uid_detail(const uid_detail& sp_detail, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_card(const token_card& mit, const short_hash& key,
                const output_point& outpoint, uint32_t output_height, uint64_t value,
                const std::string from_uid, std::string to_uid);

   class asset_visitor : public boost::static_visitor<void>
    {
    public:
        asset_visitor(data_base* db, const short_hash& sh_hash,  const output_point& outpoint,
            uint32_t output_height, uint64_t value, const std::string from_uid, std::string to_uid):
            db_(db), sh_hash_(sh_hash), outpoint_(outpoint), output_height_(output_height), value_(value),
            from_uid_(from_uid), to_uid_(to_uid)
        {

        }
        void operator()(const token &t) const
        {
            return db_->push_token(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const token_cert &t) const
        {
            return db_->push_token_cert(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const ucn &t) const
        {
            return db_->push_ucn(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const ucn_award &t) const
        {
            return db_->push_ucn_award(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const chain::blockchain_message &t) const
        {
            return db_->push_message(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const uid &t) const
        {
            return db_->push_uid(t, sh_hash_, outpoint_, output_height_, value_);
        }
        void operator()(const token_card &t) const
        {
            return db_->push_card(t, sh_hash_, outpoint_, output_height_, value_, from_uid_, to_uid_);
        }
    private:
        data_base* db_;
        short_hash sh_hash_;
        output_point outpoint_;
        uint32_t output_height_;
        uint64_t value_;
        std::string from_uid_;
        std::string to_uid_;
    };

    class token_visitor : public boost::static_visitor<void>
    {
    public:
        token_visitor(data_base* db, const short_hash& key,
            const output_point& outpoint, uint32_t output_height, uint64_t value):
            db_(db), key_(key), outpoint_(outpoint), output_height_(output_height), value_(value)
        {

        }
        void operator()(const token_detail &t) const
        {
            return db_->push_token_detail(t, key_, outpoint_, output_height_, value_);
        }
        void operator()(const token_transfer &t) const
        {
             return db_->push_token_transfer(t, key_, outpoint_, output_height_, value_);
        }
    private:
        data_base* db_;
        short_hash key_;
        output_point outpoint_;
        uint32_t output_height_;
        uint64_t value_;
    };

    void set_admin(const std::string& name, const std::string& passwd);
    void set_blackhole_uid();
    void set_token_block();
    void set_token_vote();
   /* begin store token info into  database */

protected:
    data_base(const store& paths, size_t history_height, size_t stealth_height);
    data_base(const path& prefix, size_t history_height, size_t stealth_height);

private:
    typedef chain::input::list inputs;
    typedef chain::output::list outputs;
    typedef std::atomic<size_t> sequential_lock;
    typedef boost::interprocess::file_lock file_lock;

    static bool initialize_uids(const path& prefix);
    static bool initialize_tokens(const path& prefix);
    static bool initialize_certs(const path& prefix);
    static bool initialize_cards(const path& prefix);

    static void uninitialize_lock(const path& lock);
    static file_lock initialize_lock(const path& lock);

    void synchronize();
    void synchronize_uids();
    void synchronize_certs();
    void synchronize_cards();

    void push_inputs(const hash_digest& tx_hash, size_t height,
        const inputs& inputs);
    void push_outputs(const hash_digest& tx_hash, size_t height,
        const outputs& outputs);
    void push_stealth(const hash_digest& tx_hash, size_t height,
        const outputs& outputs);
    void pop_inputs(const inputs& inputs, size_t height);
    void pop_outputs(const outputs& outputs, size_t height);

    const path lock_file_path_;
    const size_t history_height_;
    const size_t stealth_height_;

    // Atomic counter for implementing the sequential lock pattern.
    sequential_lock sequential_lock_;

    // Allows us to restrict database access to our process (or fail).
    std::shared_ptr<file_lock> file_lock_;

    // Cross-database mutext to prevent concurrent file remapping.
    std::shared_ptr<shared_mutex> mutex_;

    // temp block timestamp
    uint32_t timestamp_;

public:

    /// Individual database query engines.
    block_database blocks;
    history_database history;
    spend_database spends;
    stealth_database stealth;
    transaction_database transactions;
    /* begin database for account, token, address_token,uid relationship */
    account_database accounts;
    blockchain_token_database tokens;
    address_token_database address_tokens;
    account_token_database account_tokens;
    blockchain_token_cert_database certs;
    blockchain_uid_database uids;
    address_uid_database address_uids;
    account_address_database account_addresses;
    /* end database for account, token, address_token relationship */
    blockchain_card_database mits;
    address_card_database address_cards;
    card_history_database card_history;
};

} // namespace database
} // namespace libbitcoin

#endif

