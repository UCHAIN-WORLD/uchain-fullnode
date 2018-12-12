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
#include <UChain/database/data_base.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChainService/txs/utility/path.hpp>
#include <UChain/bitcoin/config/base16.hpp>  // used by db_metadata and push_asset
#include <UChain/database/memory/memory_map.hpp>
#include <UChain/database/settings.hpp>
#include <UChain/database/version.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;
using namespace bc::chain;
using namespace bc::wallet;
using namespace libbitcoin::config;

// BIP30 exception blocks.
// github.com/bitcoin/bips/blob/master/bip-0030.mediawiki#specification
static const config::checkpoint exception1 =
{ "00000000000a4d0a398161ffc163c503763b1f4360639393e0e4c8e300e0caec", 91842 };
static const config::checkpoint exception2 =
{ "00000000000743f190a18c5577a3c2d2a1f610ae9601ac046a38084ccb7cd721", 91880 };

bool data_base::touch_file(const path& file_path)
{
    bc::ofstream file(file_path.string());
    if (file.bad())
        return false;

    // Write one byte so file is nonzero size.
    file.write("X", 1);
    return true;
}

bool data_base::initialize(const path& prefix, const chain::block& genesis)
{
    // Create paths.
    const store paths(prefix);

    if (!paths.touch_all())
        return false;

    data_base instance(paths, 0, 0);

    if (!instance.create()) {
        return false;
    }
    auto metadata_path = prefix / db_metadata::file_name;
    auto metadata = db_metadata(db_metadata::current_version);
    data_base::write_metadata(metadata_path, metadata);
    instance.push(genesis);
    return instance.stop();
}

bool data_base::initialize_uids(const path& prefix)
{
    const store paths(prefix);
    if (paths.uids_exist())
        return true;
    if (!paths.touch_uids())
        return false;

    data_base instance(prefix, 0, 0);
    if (!instance.create_uids())
        return false;

    instance.set_blackhole_uid();

    log::info(LOG_DATABASE)
        << "Upgrading uid table is complete.";

    return instance.stop();
}

bool data_base::initialize_tokens(const path& prefix)
{
    const store paths(prefix);
    if (paths.tokens_exist())
        return true;
    if (!paths.touch_tokens())
        return false;

    data_base instance(prefix, 0, 0);
    if (!instance.create_tokens())
        return false;

    instance.set_token_block();
    instance.set_token_vote();

    log::info(LOG_DATABASE)
        << "Upgrading token table is complete.";

    return instance.stop();
}

bool data_base::initialize_certs(const path& prefix)
{
    const store paths(prefix);
    if (paths.certs_exist())
        return true;
    if (!paths.touch_certs())
        return false;

    data_base instance(prefix, 0, 0);
    if (!instance.create_certs())
        return false;

    log::info(LOG_DATABASE)
        << "Upgrading cert table is complete.";

    return instance.stop();
}

bool data_base::initialize_cards(const path& prefix)
{
    const store paths(prefix);
    if (paths.mits_exist())
        return true;
    if (!paths.touch_cards())
        return false;

    data_base instance(prefix, 0, 0);
    if (!instance.create_cards())
        return false;

    log::info(LOG_DATABASE)
        << "Upgrading card table is complete.";

    return instance.stop();
}

bool data_base::upgrade_version_63(const path& prefix)
{
    auto metadata_path = prefix / db_metadata::file_name;
    if (!boost::filesystem::exists(metadata_path))
        return false;

    data_base::db_metadata metadata;
    data_base::read_metadata(metadata_path, metadata);
    if (metadata.version_.empty()) {
        return false; // no version before, initialize all intead of upgrade.
    }

    if (!initialize_uids(prefix)) {
        log::error(LOG_DATABASE)
            << "Failed to upgrade uid database.";
        return false;
    }

    if (!initialize_certs(prefix)) {
        log::error(LOG_DATABASE)
            << "Failed to upgrade cert database.";
        return false;
    }

    if (!initialize_cards(prefix)) {
        log::error(LOG_DATABASE)
            << "Failed to upgrade card database.";
        return false;
    }

    if (metadata.version_ != db_metadata::current_version) {
        // write new db version to metadata
        metadata = db_metadata(db_metadata::current_version);
        data_base::write_metadata(metadata_path, metadata);
    }

    return true;
}

void data_base::set_admin(const std::string& name, const std::string& passwd)
{
    accounts.set_admin(name, passwd);
}

void data_base::set_blackhole_uid()
{
    const std::string uid_symbol = uid_detail::get_blackhole_uid_symbol();
    const std::string& uid_address = wallet::payment_address::blackhole_address;
    uid_detail uiddetail(uid_symbol, uid_address);

    data_chunk data(uid_address.begin(), uid_address.end());
    short_hash hash = ripemd160_hash(data);

    output_point outpoint = { null_hash, max_uint32 };
    uint32_t output_height = max_uint32;
    uint64_t value = 0;

    push_uid_detail(uiddetail, hash, outpoint, output_height, value);
    synchronize_uids();
}

void data_base::set_token_block()
{
    const std::string& uid_address = wallet::payment_address::blackhole_address;
    token_detail tokendetail(
    UC_BLOCK_TOKEN_SYMBOL, 1,
    1, 0, uid_detail::get_blackhole_uid_symbol(),
    wallet::payment_address::blackhole_address, "BLOCK token is issued by blackhole,just for name not to use");

    data_chunk data(uid_address.begin(), uid_address.end());
    short_hash hash = ripemd160_hash(data);

    output_point outpoint = { null_hash, max_uint32 };
    uint32_t output_height = max_uint32;
    uint64_t value = 0;

    push_token_detail(tokendetail, hash, outpoint, output_height, value);
    synchronize_uids();
}

void data_base::set_token_vote()
{
    const std::string& uid_address = wallet::payment_address::blackhole_address;
    token_detail tokendetail(
    UC_VOTE_TOKEN_SYMBOL, 1,
    1, 0, uid_detail::get_blackhole_uid_symbol(),
    wallet::payment_address::blackhole_address, "Vote token is issued by blackhole,just for name not to use");

    data_chunk data(uid_address.begin(), uid_address.end());
    short_hash hash = ripemd160_hash(data);

    output_point outpoint = { null_hash, max_uint32 };
    uint32_t output_height = max_uint32;
    uint64_t value = 0;

    push_token_detail(tokendetail, hash, outpoint, output_height, value);
    synchronize_uids();
}

data_base::store::store(const path& prefix)
{
    // Hash-based lookup (hash tables).
    blocks_lookup = prefix / "block_table";
    history_lookup = prefix / "history_table";
    spends_lookup = prefix / "spend_table";
    transactions_lookup = prefix / "transaction_table";
    /* begin database for account, token, address_token relationship */
    accounts_lookup = prefix / "account_table";
    tokens_lookup = prefix / "token_table";  // for blockchain tokens
    certs_lookup = prefix / "cert_table";   // for blockchain certs
    address_tokens_lookup = prefix / "address_token_table"; // for blockchain
    address_tokens_rows = prefix / "address_token_row"; // for blockchain
    account_tokens_lookup = prefix / "account_token_table";
    account_tokens_rows = prefix / "account_token_row";
    uids_lookup = prefix / "uid_table";
    address_uids_lookup = prefix / "address_uid_table"; // for blockchain
    address_uids_rows = prefix / "address_uid_row"; // for blockchain
    account_addresses_lookup = prefix / "account_address_table";
    account_addresses_rows = prefix / "account_address_rows";
    /* end database for account, token, address_token relationship */
    mits_lookup = prefix / "card_table";
    address_cards_lookup = prefix / "address_card_table"; // for blockchain
    address_cards_rows = prefix / "address_card_row"; // for blockchain
    card_history_lookup = prefix / "card_history_table"; // for blockchain
    card_history_rows = prefix / "card_history_row"; // for blockchain

    // Height-based (reverse) lookup.
    blocks_index = prefix / "block_index";

    // One (address) to many (rows).
    history_rows = prefix / "history_rows";
    stealth_rows = prefix / "stealth_rows";

    // Exclusive database access reserved by this process.
    database_lock = prefix / "process_lock";
}

bool data_base::store::touch_all() const
{
    // Return the result of the database file create.
    return
        touch_file(blocks_lookup) &&
        touch_file(blocks_index) &&
        touch_file(history_lookup) &&
        touch_file(history_rows) &&
        touch_file(stealth_rows) &&
        touch_file(spends_lookup) &&
        touch_file(transactions_lookup) &&
        /* begin database for account, token, address_token relationship */
        touch_file(accounts_lookup) &&
        touch_file(tokens_lookup) &&
        touch_file(certs_lookup) &&
        touch_file(address_tokens_lookup) &&
        touch_file(address_tokens_rows) &&
        touch_file(account_tokens_lookup) &&
        touch_file(account_tokens_rows) &&
        touch_file(uids_lookup) &&
        touch_file(address_uids_lookup) &&
        touch_file(address_uids_rows) &&
        touch_file(account_addresses_lookup) &&
        touch_file(account_addresses_rows) &&
        /* end database for account, token, address_token relationship */
        touch_file(mits_lookup) &&
        touch_file(address_cards_lookup) &&
        touch_file(address_cards_rows) &&
        touch_file(card_history_lookup) &&
        touch_file(card_history_rows);
}

bool data_base::store::uids_exist() const
{
    return
        boost::filesystem::exists(uids_lookup) ||
        boost::filesystem::exists(address_uids_lookup) ||
        boost::filesystem::exists(address_uids_rows);
}

bool data_base::store::touch_uids() const
{
    return
        touch_file(uids_lookup) &&
        touch_file(address_uids_lookup) &&
        touch_file(address_uids_rows);
}

bool data_base::store::tokens_exist() const
{
    return
        boost::filesystem::exists(tokens_lookup) ||
        boost::filesystem::exists(address_tokens_lookup) ||
        boost::filesystem::exists(address_tokens_rows);
}

bool data_base::store::touch_tokens() const
{
    return
        touch_file(tokens_lookup) &&
        touch_file(address_tokens_lookup) &&
        touch_file(address_tokens_rows);
}

bool data_base::store::certs_exist() const
{
    return boost::filesystem::exists(certs_lookup);
}

bool data_base::store::touch_certs() const
{
    return touch_file(certs_lookup);
}

bool data_base::store::mits_exist() const
{
    return
        boost::filesystem::exists(mits_lookup) ||
        boost::filesystem::exists(address_cards_lookup) ||
        boost::filesystem::exists(address_cards_rows) ||
        boost::filesystem::exists(card_history_lookup) ||
        boost::filesystem::exists(card_history_rows);
}

bool data_base::store::touch_cards() const
{
    return
        touch_file(mits_lookup) &&
        touch_file(address_cards_lookup) &&
        touch_file(address_cards_rows) &&
        touch_file(card_history_lookup) &&
        touch_file(card_history_rows);
}

data_base::db_metadata::db_metadata():version_("")
{
}

data_base::db_metadata::db_metadata(std::string version):version_(version)
{
}

void data_base::db_metadata::reset()
{
    version_ = "";
}

bool data_base::db_metadata::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool data_base::db_metadata::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool data_base::db_metadata::from_data(reader& source)
{
    reset();
    version_ = source.read_string();
    //auto result = static_cast<bool>(source);
    return true;
}

data_chunk data_base::db_metadata::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void data_base::db_metadata::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void data_base::db_metadata::to_data(writer& sink) const
{
    sink.write_string(version_);
}

uint64_t data_base::db_metadata::serialized_size() const
{
    return sizeof(version_);
}

#ifdef UC_DEBUG
std::string data_base::db_metadata::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
        ;
    return ss.str();
}
#endif

std::istream& operator>>(std::istream& input, data_base::db_metadata& metadata)
{
    std::string hexcode;
    input >> hexcode;

    metadata.from_data(base16(hexcode));

    return input;
}

std::ostream& operator<<(std::ostream& output, const data_base::db_metadata& metadata)
{
    // tx base16 is a private encoding in bx, used to pass between commands.
    const auto bytes = metadata.to_data();
    output << base16(bytes);
    return output;
}

const std::string data_base::db_metadata::current_version = UC_DATABASE_VERSION;
const std::string data_base::db_metadata::file_name = "metadata";

data_base::file_lock data_base::initialize_lock(const path& lock)
{
    // Touch the lock file to ensure its existence.
    const auto lock_file_path = lock.string();
    bc::ofstream file(lock_file_path, std::ios::app);
    file.close();
#ifdef UNICODE
    std::function<std::string(std::wstring)> f = [&](std::wstring wide) ->std::string {
        int ansiiLen = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        char *pAssii = new char[ansiiLen];
        WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
        std::string str(pAssii);
        delete[] pAssii;
        return str;
    };
    std::string path_str = f(lock.wstring());
#else
    std::string path_str = lock_file_path;
#endif
    // BOOST:
    // Opens a file lock. Throws interprocess_exception if the file does not
    // exist or there are no operating system resources. The file lock is
    // destroyed on its destruct and does not throw.
    return file_lock(path_str.c_str());
}

void data_base::uninitialize_lock(const path& lock)
{
    // BUGBUG: Throws if the lock is not held (i.e. in error condition).
    boost::filesystem::remove(lock);
}

data_base::data_base(const settings& settings)
  : data_base(settings.directory, settings.history_start_height,
        settings.stealth_start_height)
{
}

data_base::data_base(const path& prefix, size_t history_height,
    size_t stealth_height)
  : data_base(store(prefix), history_height, stealth_height)
{
}

data_base::data_base(const store& paths, size_t history_height,
    size_t stealth_height)
  : lock_file_path_(paths.database_lock),
    history_height_(history_height),
    stealth_height_(stealth_height),
    sequential_lock_(0),
    mutex_(std::make_shared<shared_mutex>()),
    blocks(paths.blocks_lookup, paths.blocks_index, mutex_),
    history(paths.history_lookup, paths.history_rows, mutex_),
    stealth(paths.stealth_rows, mutex_),
    spends(paths.spends_lookup, mutex_),
    transactions(paths.transactions_lookup, mutex_),
    /* begin database for account, token, address_token, uid relationship */
    accounts(paths.accounts_lookup, mutex_),
    tokens(paths.tokens_lookup, mutex_),
    address_tokens(paths.address_tokens_lookup, paths.address_tokens_rows, mutex_),
    account_tokens(paths.account_tokens_lookup, paths.account_tokens_rows, mutex_),
    certs(paths.certs_lookup, mutex_),
    uids(paths.uids_lookup, mutex_),
    address_uids(paths.address_uids_lookup, paths.address_uids_rows, mutex_),
    account_addresses(paths.account_addresses_lookup, paths.account_addresses_rows, mutex_),
    /* end database for account, token, address_token, uid relationship */
    cards(paths.mits_lookup, mutex_),
    address_cards(paths.address_cards_lookup, paths.address_cards_rows, mutex_),
    card_history(paths.card_history_lookup, paths.card_history_rows, mutex_)
{
}

// Close does not call stop because there is no way to detect thread join.
data_base::~data_base()
{
    close();
}

void data_base::write_metadata(const path& metadata_path, data_base::db_metadata& metadata)
{
    bc::ofstream file_output(metadata_path.string(), std::ofstream::out);
    file_output << metadata;
    file_output << std::flush;
    file_output.close();
}

void data_base::read_metadata(const path& metadata_path, data_base::db_metadata& metadata)
{
    if(!boost::filesystem::exists(metadata_path)) {
        metadata = data_base::db_metadata();
        return;
    }
    bc::ifstream file_input(metadata_path.string(), std::ofstream::in);
    if (!file_input.good()) {
        throw std::logic_error{std::string("read_metadata error : ")+ strerror(errno)};
    }
    file_input >> metadata;
    file_input.close();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Leaves database in started state.
// Throws if there is insufficient disk space.
// TODO: merge this with file creation (initialization above).
// This is actually first initialization of existing files, not file creation.
bool data_base::create()
{
    // Return the result of the database create.
    return
        blocks.create() &&
        history.create() &&
        spends.create() &&
        stealth.create() &&
        transactions.create() &&
        /* begin database for account, token, address_token relationship */
        accounts.create() &&
        tokens.create() &&
        address_tokens.create() &&
        account_tokens.create() &&
        certs.create() &&
        uids.create() &&
        address_uids.create() &&
        account_addresses.create() &&
        /* end database for account, token, address_token relationship */
        cards.create() &&
        address_cards.create() &&
        card_history.create()
        ;
}

bool data_base::create_uids()
{
    return
        uids.create() &&
        address_uids.create();
}

bool data_base::create_tokens()
{
    return
        tokens.create() &&
        address_tokens.create();
}

bool data_base::create_certs()
{
    return
        certs.create();
}

bool data_base::create_cards()
{
    return
        cards.create() &&
        address_cards.create() &&
        card_history.create();
}

// Start must be called before performing queries.
// Start may be called after stop and/or after close in order to restart.
bool data_base::start()
{
    // TODO: create a class to encapsulate the full file lock concept.
    file_lock_ = std::make_shared<file_lock>(initialize_lock(lock_file_path_));

    // BOOST:
    // Effects: The calling thread tries to acquire exclusive ownership of the
    // mutex without waiting. If no other thread has exclusive, or sharable
    // ownership of the mutex this succeeds. Returns: If it can acquire
    // exclusive ownership immediately returns true. If it has to wait, returns
    // false. Throws: interprocess_exception on error. Note that a file lock
    // can't guarantee synchronization between threads of the same process so
    // just use file locks to synchronize threads from different processes.
    if (!file_lock_->try_lock())
        return false;

    const auto start_exclusive = begin_write();
    const auto start_result =
        blocks.start() &&
        history.start() &&
        spends.start() &&
        stealth.start() &&
        transactions.start() &&
        /* begin database for account, token, address_token relationship */
        accounts.start() &&
        tokens.start() &&
        address_tokens.start() &&
        account_tokens.start() &&
        certs.start() &&
        uids.start() &&
        address_uids.start() &&
        account_addresses.start() &&
        /* end database for account, token, address_token relationship */
        cards.start() &&
        address_cards.start() &&
        card_history.start()
        ;
    const auto end_exclusive = end_write();

    // Return the result of the database start.
    return start_exclusive && start_result && end_exclusive;
}

// Stop only accelerates work termination, only required if restarting.
bool data_base::stop()
{
    const auto start_exclusive = begin_write();
    const auto blocks_stop = blocks.stop();
    const auto history_stop = history.stop();
    const auto spends_stop = spends.stop();
    const auto stealth_stop = stealth.stop();
    const auto transactions_stop = transactions.stop();
    /* begin database for account, token, address_token relationship */
    const auto accounts_stop = accounts.stop();
    const auto tokens_stop = tokens.stop();
    const auto address_tokens_stop = address_tokens.stop();
    const auto account_tokens_stop = account_tokens.stop();
    const auto certs_stop = certs.stop();
    const auto uids_stop = uids.stop();
    const auto address_uids_stop = address_uids.stop();
    const auto account_addresses_stop = account_addresses.stop();
    /* end database for account, token, address_token relationship */
    const auto mits_stop = cards.stop();
    const auto address_cards_stop = address_cards.stop();
    const auto card_history_stop = card_history.stop();
    const auto end_exclusive = end_write();

    // This should remove the lock file. This is not important for locking
    // purposes, but it provides a sentinel to indicate hard shutdown.
    file_lock_ = nullptr;
    uninitialize_lock(lock_file_path_);

    // Return the cumulative result of the database shutdowns.
    return
        start_exclusive &&
        blocks_stop &&
        history_stop &&
        spends_stop &&
        stealth_stop &&
        transactions_stop &&
        /* begin database for account, token, address_token relationship */
        accounts_stop &&
        tokens_stop &&
        address_tokens_stop &&
        account_tokens_stop &&
        certs_stop &&
        uids_stop &&
        address_uids_stop &&
        account_addresses_stop &&
        /* end database for account, token, address_token relationship */
        mits_stop &&
        address_cards_stop &&
        card_history_stop &&
        end_exclusive;
}

// Close is optional as the database will close on destruct.
bool data_base::close()
{
    const auto blocks_close = blocks.close();
    const auto history_close = history.close();
    const auto spends_close = spends.close();
    const auto stealth_close = stealth.close();
    const auto transactions_close = transactions.close();
    /* begin database for account, token, address_token relationship */
    const auto accounts_close = accounts.close();
    const auto tokens_close = tokens.close();
    const auto address_tokens_close = address_tokens.close();
    const auto address_uids_close = address_uids.close();
    const auto account_tokens_close = account_tokens.close();
    const auto certs_close = certs.close();
    const auto uids_close = uids.close();
    const auto account_addresses_close = account_addresses.close();
    /* end database for account, token, address_token relationship */
    const auto mits_close = cards.close();
    const auto address_cards_close = address_cards.close();
    const auto card_history_close = card_history.close();

    // Return the cumulative result of the database closes.
    return
        blocks_close &&
        history_close &&
        spends_close &&
        stealth_close &&
        transactions_close&&
        /* begin database for account, token, address_token relationship */
        accounts_close &&
        tokens_close &&
        address_tokens_close&&
        address_uids_close &&
        account_tokens_close&&
        certs_close &&
        uids_close &&
        account_addresses_close &&
        /* end database for account, token, address_token relationship */
        mits_close &&
        address_cards_close &&
        card_history_close
        ;
}

// Locking.
// ----------------------------------------------------------------------------

handle data_base::begin_read()
{
    return sequential_lock_.load();
}

bool data_base::is_read_valid(handle value)
{
    return value == sequential_lock_.load();
}

bool data_base::is_write_locked(handle value)
{
    return (value % 2) == 1;
}

// TODO: drop a file as a write sentinel that we can use to detect uncontrolled
// shutdown during write. Use a similar approach around initial block download.
// Fail startup if the sentinel is detected. (file: write_lock).
bool data_base::begin_write()
{
    // slock is now odd.
    return is_write_locked(++sequential_lock_);
}

// TODO: clear the write sentinel.
bool data_base::end_write()
{
    // slock_ is now even again.
    return !is_write_locked(++sequential_lock_);
}

// Query engines.
// ----------------------------------------------------------------------------

static size_t get_next_height(const block_database& blocks)
{
    size_t current_height;
    const auto empty_chain = !blocks.top(current_height);
    return empty_chain ? 0 : current_height + 1;
}

static bool is_allowed_duplicate(const header& head, size_t height)
{
    return
        (height == exception1.height() && head.hash() == exception1.hash()) ||
        (height == exception2.height() && head.hash() == exception2.hash());
}

void data_base::synchronize()
{
    spends.sync();
    history.sync();
    stealth.sync();
    transactions.sync();
    /* begin database for account, token, address_token relationship */
    accounts.sync();
    tokens.sync();
    address_tokens.sync();
    account_tokens.sync();
    certs.sync();
    uids.sync();
    address_uids.sync();
    account_addresses.sync();
    /* end database for account, token, address_token relationship */
    cards.sync();
    address_cards.sync();
    card_history.sync();
    blocks.sync();
}

void data_base::synchronize_uids()
{
    uids.sync();
    address_uids.sync();
}

void data_base::synchronize_certs()
{
    certs.sync();
}

void data_base::synchronize_cards()
{
    cards.sync();
    address_cards.sync();
    card_history.sync();
}

void data_base::push(const block& block)
{
    // Height is unsafe unless database locked.
    push(block, get_next_height(blocks));
}

void data_base::push(const block& block, uint64_t height)
{
    for (size_t index = 0; index < block.transactions.size(); ++index)
    {
        // Skip BIP30 allowed duplicates (coinbase txs of excepted blocks).
        // We handle here because this is the lowest public level exposed.
        if (index == 0 && is_allowed_duplicate(block.header, height))
            continue;

        const auto& tx = block.transactions[index];
        const auto tx_hash = tx.hash();

        timestamp_ = block.header.timestamp; // for address_token_database store_input/store_output used only
        // Add inputs
        if (!tx.is_strict_coinbase())
            push_inputs(tx_hash, height, tx.inputs);

        // std::string uidaddress = tx.get_uid_transfer_old_address();
        // if (!uidaddress.empty()) {
        //     data_chunk data(uidaddress.begin(), uidaddress.end());
        //     short_hash key = ripemd160_hash(data);
        //     address_uids.delete_old_uid(key);
        // }

        // Add outputs
        push_outputs(tx_hash, height, tx.outputs);

        // Add stealth outputs
        push_stealth(tx_hash, height, tx.outputs);

        // Add transaction
        transactions.store(height, index, tx);
    }

    // Add block itself.
    blocks.store(block, height);

    // Synchronise everything that was added.
    synchronize();
}

void data_base::push_inputs(const hash_digest& tx_hash, size_t height,
    const input::list& inputs)
{
    for (uint32_t index = 0; index < inputs.size(); ++index)
    {
        // We also push spends in the inputs loop.
        const auto& input = inputs[index];
        const chain::input_point point{ tx_hash, index };
        spends.store(input.previous_output, point);

        if (height < history_height_)
            continue;

        // Try to extract an address.
        const auto address = payment_address::extract(input.script);
        if (!address)
            continue;

        const auto& previous = input.previous_output;
        history.add_input(address.hash(), point, height, previous);

        /* begin added for token issue/transfer */
        auto address_str = address.encoded();
        data_chunk data(address_str.begin(), address_str.end());
        short_hash key = ripemd160_hash(data);
        address_tokens.store_input(key, point, height, previous, timestamp_);
        address_tokens.sync();
        /* end added for token issue/transfer */
    }
}

void data_base::push_outputs(const hash_digest& tx_hash, size_t height,
    const output::list& outputs)
{
    if (height < history_height_)
        return;

    for (uint32_t index = 0; index < outputs.size(); ++index)
    {
        const auto& output = outputs[index];
        const chain::output_point point{ tx_hash, index };

        // Try to extract an address.
        const auto address = payment_address::extract(output.script);
        if (!address)
            continue;

        const auto value = output.value;
        history.add_output(address.hash(), point, height, value);

        push_asset(output.attach_data, address, point, height, value);
    }
}

void data_base::push_stealth(const hash_digest& tx_hash, size_t height,
    const output::list& outputs)
{
    if (height < stealth_height_ || outputs.empty())
        return;

    // Stealth outputs are paired by convention.
    for (size_t index = 0; index < (outputs.size() - 1); ++index)
    {
        const auto& ephemeral_script = outputs[index].script;
        const auto& payment_script = outputs[index + 1].script;

        // Try to extract an unsigned ephemeral key from the first output.
        hash_digest unsigned_ephemeral_key;
        if (!extract_ephemeral_key(unsigned_ephemeral_key, ephemeral_script))
            continue;

        // Try to extract a stealth prefix from the first output.
        uint32_t prefix;
        if (!to_stealth_prefix(prefix, ephemeral_script))
            continue;

        // Try to extract the payment address from the second output.
        const auto address = payment_address::extract(payment_script);
        if (!address)
            continue;

        // The payment address versions are arbitrary and unused here.
        const chain::stealth_compact row
        {
            unsigned_ephemeral_key,
            address.hash(),
            tx_hash
        };

        stealth.store(prefix, height, row);
    }
}

chain::block data_base::pop()
{
    size_t height;
    DEBUG_ONLY(const auto result =) blocks.top(height);
    BITCOIN_ASSERT_MSG(result, "Pop on empty database.");

    const auto block_result = blocks.get(height);
    const auto count = block_result.transaction_count();

    // Build the block for return.
    chain::block block;
    block.header = block_result.header();
    block.transactions.reserve(count);
    auto& txs = block.transactions;

    for (size_t tx = 0; tx < count; ++tx)
    {
        const auto tx_hash = block_result.transaction_hash(tx);
        const auto tx_result = transactions.get(tx_hash);

        //fix a bug ,synchronize block may destroy the database
        /*if (!tx_result) {
            continue;
        }*/

        BITCOIN_ASSERT(tx_result);
        BITCOIN_ASSERT(tx_result.height() == height);
        BITCOIN_ASSERT(tx_result.index() == static_cast<size_t>(tx));

        // TODO: the deserialization should cache the hash on the tx.
        // Deserialize the transaction and move it to the block.
        txs.emplace_back(tx_result.transaction());
    }

    // Loop txs backwards, the reverse of how they are added.
    // Remove txs, then outputs, then inputs (also reverse order).
    for (auto tx = txs.rbegin(); tx != txs.rend(); ++tx)
    {
        transactions.remove(tx->hash());
        pop_outputs(tx->outputs, height);

        if (!tx->is_strict_coinbase())
            pop_inputs(tx->inputs, height);
    }

    // Stealth unlink is not implemented.
    stealth.unlink(height);
    blocks.unlink(height);
    blocks.remove(block.header.hash()); // wdy remove block from block hash table

    // Synchronise everything that was changed.
    synchronize();

    // Return the block.
    return block;
}

void data_base::pop_inputs(const input::list& inputs, size_t height)
{
    // Loop in reverse.
    for (auto input = inputs.rbegin(); input != inputs.rend(); ++input)
    {
        spends.remove(input->previous_output);

        if (height < history_height_)
            continue;

        // Try to extract an address.
        const auto address = payment_address::extract(input->script);

        if (address) {
            history.delete_last_row(address.hash());
            // delete address token record
            auto address_str = address.encoded();
            data_chunk data(address_str.begin(), address_str.end());
            short_hash hash = ripemd160_hash(data);
            address_tokens.delete_last_row(hash);
        }
    }
}

void data_base::pop_outputs(const output::list& outputs, size_t height)
{
    if (height < history_height_)
        return;

    // Loop in reverse.
    for (auto output = outputs.rbegin(); output != outputs.rend(); ++output)
    {
        // Try to extract an address.
        const auto address = payment_address::extract(output->script);

        if (address) {
            history.delete_last_row(address.hash());
            // delete address token record
            auto address_str = address.encoded();
            data_chunk data(address_str.begin(), address_str.end());
            short_hash hash = ripemd160_hash(data);
            bc::chain::output op = *output;
            // NOTICE: pop only the pushed row, at present uid and card is
            // not stored in address_token, but stored separately
            // in address_uid and address_uid
            if (!op.is_uid() && !op.is_token_card()) {
                address_tokens.delete_last_row(hash);
            }
            // remove token or uid from database
            if (op.is_token_issue() || op.is_token_secondaryissue()) {
                auto symbol = op.get_token_symbol();
                const data_chunk& symbol_data = data_chunk(symbol.begin(), symbol.end());
                const auto symbol_hash = sha256_hash(symbol_data);
                tokens.remove(symbol_hash);
            }
            else if (op.is_uid()) {
                auto symbol = op.get_uid_symbol();
                const data_chunk& symbol_data = data_chunk(symbol.begin(), symbol.end());
                const auto symbol_hash = sha256_hash(symbol_data);

                if(op.is_uid_register())
                {
                    address_uids.delete_last_row(hash);
                    address_uids.sync();
                    uids.remove(symbol_hash);
                    uids.sync();
                }
                else if(op.is_uid_transfer() )
                {
                    std::shared_ptr<blockchain_uid> blockchain_uid_=  uids.pop_uid_transfer(symbol_hash);
                    uids.sync();

                    if(blockchain_uid_)
                    {
                        auto old_address = blockchain_uid_->get_uid().get_address();
                        data_chunk data_old(old_address.begin(), old_address.end());
                        short_hash old_hash = ripemd160_hash(data_old);

                        address_uids.delete_last_row(old_hash);
                        address_uids.delete_last_row(hash);

                        address_uids.store_output(old_hash, blockchain_uid_->get_tx_point(), blockchain_uid_->get_height(), 0,
                            static_cast<typename std::underlying_type<business_kind>::type>(business_kind::uid_register),
                            timestamp_, blockchain_uid_->get_uid());
                        address_uids.sync();

                    }
                }

            }
            else if (op.is_token_cert()) {
                const auto token_cert = op.get_token_cert();
                if (token_cert.is_newly_generated()) {
                    const auto key_str = token_cert.get_key();
                    const data_chunk& data = data_chunk(key_str.begin(), key_str.end());
                    const auto key_hash = sha256_hash(data);
                    certs.remove(key_hash);
                }
            }
            else if (op.is_token_card()) {
                address_cards.delete_last_row(hash);

                const auto card = op.get_token_card();
                auto symbol = card.get_symbol();
                const data_chunk& symbol_data = data_chunk(symbol.begin(), symbol.end());

                const auto symbol_short_hash = ripemd160_hash(symbol_data);
                card_history.delete_last_row(symbol_short_hash);

                if (card.is_register_status()) {
                    const auto symbol_hash = sha256_hash(symbol_data);
                    cards.remove(symbol_hash);
                }
            }
        }
    }
}

/* begin store token related info into database */
#include <UChain/bitcoin/config/base16.hpp>
using namespace libbitcoin::config;

void data_base::push_asset(const asset& attach, const payment_address& address,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    auto address_str = address.encoded();
    log::trace(LOG_DATABASE) << "push_asset address_str=" << address_str;
    log::trace(LOG_DATABASE) << "push_asset address hash=" << base16(address.hash());
    data_chunk data(address_str.begin(), address_str.end());
    short_hash hash = ripemd160_hash(data);
    auto visitor = asset_visitor(this, hash, outpoint, output_height, value,
        attach.get_from_uid(), attach.get_to_uid());
    boost::apply_visitor(visitor, const_cast<asset&>(attach).get_attach());
}

void data_base::push_ucn(const ucn& ucn, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::ucn),
        timestamp_, ucn);
    address_tokens.sync();

}

void data_base::push_ucn_award(const ucn_award& award, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::ucn_award),
        timestamp_, award);
    address_tokens.sync();
}

void data_base::push_message(const chain::blockchain_message& msg, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::message),
        timestamp_, msg);
    address_tokens.sync();
}

void data_base::push_token(const token& sp, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value) // sp = smart property
{
    auto visitor = token_visitor(this, key, outpoint, output_height, value);
    boost::apply_visitor(visitor, const_cast<token&>(sp).get_data());
}

void data_base::push_token_cert(const token_cert& sp_cert, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    if (sp_cert.is_newly_generated()) {
        certs.store(sp_cert);
        certs.sync();
    }
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::token_cert),
        timestamp_, sp_cert);
    address_tokens.sync();
}

void data_base::push_token_detail(const token_detail& sp_detail, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    const data_chunk& data = data_chunk(sp_detail.get_symbol().begin(), sp_detail.get_symbol().end());
    const auto hash = sha256_hash(data);
    auto bc_token = blockchain_token(0, outpoint,output_height, sp_detail);
    tokens.store(hash, bc_token);
    tokens.sync();
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::token_issue),
        timestamp_, sp_detail);
    address_tokens.sync();
}

void data_base::push_token_transfer(const token_transfer& sp_transfer, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    address_tokens.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::token_transfer),
        timestamp_, sp_transfer);
    address_tokens.sync();
}
/* end store token related info into database */

/* begin store uid related info into database */
void data_base::push_uid(const uid& sp, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value) // sp = smart property
{
    push_uid_detail(sp.get_data(), key, outpoint, output_height, value);
}

void data_base::push_uid_detail(const uid_detail& sp_detail, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value)
{
    const data_chunk& data = data_chunk(sp_detail.get_symbol().begin(), sp_detail.get_symbol().end());
    const auto hash = sha256_hash(data);
    auto bc_uid = blockchain_uid(0, outpoint,output_height, blockchain_uid::address_current,sp_detail);
    uids.store(hash, bc_uid);
    uids.sync();
    address_uids.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::uid_register),
        timestamp_, sp_detail);
    address_uids.sync();
}

/* end store uid related info into database */

/* begin store card related info into database */
void data_base::push_card(const token_card& card, const short_hash& key,
    const output_point& outpoint, uint32_t output_height, uint64_t value,
    const std::string from_uid, std::string to_uid)
{
    token_card_info card_info{output_height, timestamp_, to_uid, card};

    if (card.is_register_status()) {
        cards.store(card_info);
        cards.sync();
    }

    address_cards.store_output(key, outpoint, output_height, value,
        static_cast<typename std::underlying_type<business_kind>::type>(business_kind::token_card),
        timestamp_, card);
    address_cards.sync();

    card_history.store(card_info);
    card_history.sync();
}
/* end store card related info into database */

} // namespace data_base
} // namespace libbitcoin

