/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS) 
 *
 * This file is part of ucd.
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
#include <UChainService/data/databases/address_uid_db.hpp>
//#include <UChainService/txs/wallet/address_uid.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/primitives/record_multimap_iterable.hpp>
#include <UChain/database/primitives/record_multimap_iterator.hpp>

#define LOG_ADDRESS_UID_DATABASE "address_uid_database"

namespace libbitcoin
{
namespace database
{

using namespace boost::filesystem;
using namespace bc::chain;

BC_CONSTEXPR size_t number_buckets = 97210744;
BC_CONSTEXPR size_t header_size = record_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_lookup_file_size = header_size + minimum_records_size;

BC_CONSTEXPR size_t record_size = hash_table_multimap_record_size<short_hash>();

BC_CONSTEXPR size_t uid_transfer_record_size = 1 + 36 + 4 + 8 + 2 + 4 + UID_DETAIL_FIX_SIZE; // UID_DETAIL_FIX_SIZE is the biggest one
//        + std::max({UCN_FIX_SIZE, UID_DETAIL_FIX_SIZE, UID_TRANSFER_FIX_SIZE});
BC_CONSTEXPR size_t row_record_size = hash_table_record_size<hash_digest>(uid_transfer_record_size);

address_uid_database::address_uid_database(const path &lookup_filename,
                                           const path &rows_filename, std::shared_ptr<shared_mutex> mutex)
    : lookup_file_(lookup_filename, mutex),
      lookup_header_(lookup_file_, number_buckets),
      lookup_manager_(lookup_file_, header_size, record_size),
      lookup_map_(lookup_header_, lookup_manager_),
      rows_file_(rows_filename, mutex),
      rows_manager_(rows_file_, 0, row_record_size),
      rows_list_(rows_manager_),
      rows_multimap_(lookup_map_, rows_list_)
{
}

// Close does not call stop because there is no way to detect thread join.
address_uid_database::~address_uid_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool address_uid_database::create()
{
    // Resize and create require a started file.
    if (!lookup_file_.start() ||
        !rows_file_.start())
        return false;

    // These will throw if insufficient disk space.
    lookup_file_.resize(initial_lookup_file_size);
    rows_file_.resize(minimum_records_size);

    if (!lookup_header_.create() ||
        !lookup_manager_.create() ||
        !rows_manager_.create())
        return false;

    // Should not call start after create, already started.
    return lookup_header_.start() &&
           lookup_manager_.start() &&
           rows_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

bool address_uid_database::start()
{
    return lookup_file_.start() &&
           rows_file_.start() &&
           lookup_header_.start() &&
           lookup_manager_.start() &&
           rows_manager_.start();
}

bool address_uid_database::stop()
{
    return lookup_file_.stop() &&
           rows_file_.stop();
}

bool address_uid_database::close()
{
    return lookup_file_.close() &&
           rows_file_.close();
}

// ----------------------------------------------------------------------------
void address_uid_database::store_input(const short_hash &key,
                                       const output_point &inpoint, uint32_t input_height,
                                       const input_point &previous, uint32_t timestamp)
{
    auto write = [&](memory_ptr data) {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_byte(static_cast<uint8_t>(point_kind::spend)); // 1
        serial.write_data(inpoint.to_data());                       // 36
        serial.write_4_bytes_little_endian(input_height);           // 4
        serial.write_8_bytes_little_endian(previous.checksum());    // 8

        serial.write_2_bytes_little_endian(0);         // 2 use ucn type fill incase invalid when deser
        serial.write_4_bytes_little_endian(timestamp); // 4
        // uid data should be here but input has no these data
    };
    rows_multimap_.add_row(key, write);
}

void address_uid_database::delete_old_uid(const short_hash &key)
{
    delete_last_row(key);
}

void address_uid_database::delete_last_row(const short_hash &key)
{
    rows_multimap_.delete_last_row(key);
}
/// get all record of key from database
business_record::list address_uid_database::get(const short_hash &key,
                                                size_t from_height, size_t limit) const
{
    // Read the height value from the row.
    const auto read_height = [](uint8_t *data) {
        static constexpr file_offset height_position = 1 + 36;
        const auto height_address = data + height_position;
        return from_little_endian_unsafe<uint32_t>(height_address);
    };

    // Read a row from the data for the history list.
    const auto read_row = [](uint8_t *data) {
        auto deserial = make_deserializer_unsafe(data);
        return business_record{
            // output or spend?
            static_cast<point_kind>(deserial.read_byte()),

            // point
            point::factory_from_data(deserial),

            // height
            deserial.read_4_bytes_little_endian(),

            // value or checksum
            {deserial.read_8_bytes_little_endian()},

            // business_kd;
            //deserial.read_2_bytes_little_endian(),
            // timestamp;
            //deserial.read_4_bytes_little_endian(),

            asset_data::factory_from_data(deserial) // 2 + 4 are in this class
        };
    };

    business_record::list result;
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    for (const auto index : records)
    {
        // Stop once we reach the limit (if specified).
        if (limit > 0 && result.size() >= limit)
            break;

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        // Skip rows below from_height.
        if (from_height == 0 || read_height(address) <= from_height) // from current block height
            result.emplace_back(read_row(address));
    }

    // TODO: we could sort result here.
    return result;
}
/// get all record of key from database
std::shared_ptr<std::vector<business_record>> address_uid_database::get(const std::string &address, const std::string &symbol,
                                                                        size_t start_height, size_t end_height, uint64_t limit, uint64_t page_number) const
{
    data_chunk addr_data(address.begin(), address.end());
    auto key = ripemd160_hash(addr_data);

    // Read the height value from the row.
    const auto read_height = [](uint8_t *data) {
        static constexpr file_offset height_position = 1 + 36;
        const auto height_address = data + height_position;
        return from_little_endian_unsafe<uint32_t>(height_address);
    };

    // Read a row from the data for the history list.
    const auto read_row = [](uint8_t *data) {
        auto deserial = make_deserializer_unsafe(data);
        return business_record{
            // output or spend?
            static_cast<point_kind>(deserial.read_byte()),

            // point
            point::factory_from_data(deserial),

            // height
            deserial.read_4_bytes_little_endian(),

            // value or checksum
            {deserial.read_8_bytes_little_endian()},

            // business_kd;
            //deserial.read_2_bytes_little_endian(),
            // timestamp;
            //deserial.read_4_bytes_little_endian(),

            asset_data::factory_from_data(deserial) // 2 + 4 are in this class
        };
    };

    auto result = std::make_shared<std::vector<business_record>>();
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    uint64_t cnt = 0;
    for (const auto index : records)
    {
        // Stop once we reach the limit (if specified).
        if (limit > 0 && result->size() >= limit)
            break;

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);
        auto height = read_height(address);
        std::string uid_symbol;

        // Skip rows below from_height.
        if (((start_height == 0) && (end_height == 0)) || ((start_height <= height) && (height < end_height)))
        { // from current block height
            //result->emplace_back(read_row(address));
            auto row = read_row(address);
            if (symbol.empty())
            { // all utxo
                cnt++;
                if ((limit > 0) && (page_number > 0) && ((cnt - 1) / limit) < (page_number - 1))
                    continue; // skip previous page record
                result->emplace_back(row);
            }
            else
            { // uid symbol utxo
                // uid business process
                uid_symbol = "";
                if (row.data.get_kind_value() == business_kind::uid_register)
                {
                    auto transfer = boost::get<uid_detail>(row.data.get_data());
                    uid_symbol = transfer.get_symbol();
                }
                else if (row.data.get_kind_value() == business_kind::uid_transfer)
                {
                    auto transfer = boost::get<uid_detail>(row.data.get_data());
                    uid_symbol = transfer.get_symbol();
                }

                if (symbol == uid_symbol)
                {
                    cnt++;
                    if ((limit > 0) && (page_number > 0) && ((cnt - 1) / limit) < (page_number - 1))
                        continue; // skip previous page record
                    result->emplace_back(row);
                }
            }
        }
    }

    // TODO: we could sort result here.
    return result;
}

/// get all record of key from database
std::shared_ptr<std::vector<business_record>> address_uid_database::get(const std::string &address, size_t start_height,
                                                                        size_t end_height) const
{
    data_chunk addr_data(address.begin(), address.end());
    auto key = ripemd160_hash(addr_data);

    // Read the height value from the row.
    const auto read_height = [](uint8_t *data) {
        static constexpr file_offset height_position = 1 + 36;
        const auto height_address = data + height_position;
        return from_little_endian_unsafe<uint32_t>(height_address);
    };

    // Read a row from the data for the history list.
    const auto read_row = [](uint8_t *data) {
        auto deserial = make_deserializer_unsafe(data);
        return business_record{
            // output or spend?
            static_cast<point_kind>(deserial.read_byte()),

            // point
            point::factory_from_data(deserial),

            // height
            deserial.read_4_bytes_little_endian(),

            // value or checksum
            {deserial.read_8_bytes_little_endian()},

            // business_kd;
            //deserial.read_2_bytes_little_endian(),
            // timestamp;
            //deserial.read_4_bytes_little_endian(),

            asset_data::factory_from_data(deserial) // 2 + 4 are in this class
        };
    };

    auto result = std::make_shared<std::vector<business_record>>();
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    for (const auto index : records)
    {
        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);
        auto height = read_height(address);
        // Skip rows below from_height.
        if (((start_height == 0) && (end_height == 0)) || ((start_height <= height) && (height < end_height))) // from current block height
            result->emplace_back(read_row(address));
    }

    // TODO: we could sort result here.
    return result;
}

/// get all record of key from database
std::shared_ptr<std::vector<business_record>> address_uid_database::get(size_t idx) const
{
    // Read a row from the data for the history list.
    const auto read_row = [](uint8_t *data) {
        auto deserial = make_deserializer_unsafe(data);
        return business_record{
            // output or spend?
            static_cast<point_kind>(deserial.read_byte()),

            // point
            point::factory_from_data(deserial),

            // height
            deserial.read_4_bytes_little_endian(),

            // value or checksum
            {deserial.read_8_bytes_little_endian()},

            // business_kd;
            //deserial.read_2_bytes_little_endian(),
            // timestamp;
            //deserial.read_4_bytes_little_endian(),

            asset_data::factory_from_data(deserial) // 2 + 4 are in this class
        };
    };

    auto result = std::make_shared<std::vector<business_record>>();
    auto sh_idx_vec = rows_multimap_.lookup(idx);

    for (auto each : *sh_idx_vec)
    {

        const auto records = record_multimap_iterable(rows_list_, each);

        for (const auto index : records)
        {
            // This obtains a remap safe address pointer against the rows file.
            const auto record = rows_list_.get(index);
            const auto address = REMAP_ADDRESS(record);
            result->emplace_back(read_row(address));
        }
    }

    // TODO: we could sort result here.
    return result;
}
/// get one record by index from row_list
business_record address_uid_database::get_record(size_t idx) const
{
    // Read a row from the data for the history list.
    const auto read_row = [](uint8_t *data) {
        auto deserial = make_deserializer_unsafe(data);
        return business_record{
            // output or spend?
            static_cast<point_kind>(deserial.read_byte()),

            // point
            point::factory_from_data(deserial),

            // height
            deserial.read_4_bytes_little_endian(),

            // value or checksum
            {deserial.read_8_bytes_little_endian()},

            // business_kd;
            //deserial.read_2_bytes_little_endian(),
            // timestamp;
            //deserial.read_4_bytes_little_endian(),

            asset_data::factory_from_data(deserial) // 2 + 4 are in this class
        };
    };

    // This obtains a remap safe address pointer against the rows file.
    const auto record = rows_list_.get(idx);
    const auto address = REMAP_ADDRESS(record);
    return read_row(address);
}
business_history::list address_uid_database::get_business_history(const short_hash &key,
                                                                  size_t from_height) const
{
    business_record::list compact = get(key, from_height, 0);

    business_history::list result;

    // Process and remove all outputs.
    for (auto output = compact.begin(); output != compact.end();)
    {
        if (output->kind == point_kind::output)
        {
            business_history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->val_chk_sum.value;
            row.spend = {null_hash, max_uint32};
            row.temporary_checksum = output->point.checksum();
            row.data = output->data;
            result.emplace_back(row);
            output = compact.erase(output);
            continue;
        }

        ++output;
    }

    // All outputs have been removed, process the spends.
    for (const auto &spend : compact)
    {
        auto found = false;

        // Update outputs with the corresponding spends.
        for (auto &row : result)
        {
            if (row.temporary_checksum == spend.val_chk_sum.previous_checksum &&
                row.spend.hash == null_hash)
            {
                row.spend = spend.point;
                row.spend_height = spend.height;
                found = true;
                break;
            }
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        if (!found)
        {
            business_history row;
            row.output = {null_hash, max_uint32};
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto &row : result)
        if (row.spend.hash == null_hash)
            row.spend_height = max_uint64;

    // TODO: sort by height and index of output, spend or both in order.
    return result;
}

// get address uids in the database(blockchain)
std::shared_ptr<std::vector<business_history>> address_uid_database::get_address_business_history(const std::string &address,
                                                                                                  size_t from_height) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    auto unspent = std::make_shared<std::vector<business_history>>();

    for (auto &row : result)
    {
        if ((row.spend.hash == null_hash))
        { // unspent business
            row.status = business_status::unspent;
            unspent->emplace_back(row);
        }

        if (row.output_height != 0 && (row.spend.hash == null_hash || row.spend_height == 0))
        { // confirmed business
            row.status = business_status::confirmed;
            unspent->emplace_back(row);
        }
    }
    return unspent;
}

// get special kind of uid in the database(blockchain)
/*
 status -- // 0 -- unspent  1 -- confirmed
*/
business_history::list address_uid_database::get_business_history(const std::string &address,
                                                                  size_t from_height, business_kind kind, uint8_t status) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    business_history::list unspent;
    // uid type check
    if ((kind != business_kind::uid_register)    // uid_detail
        && (kind != business_kind::uid_transfer) // uid_transfer
        && (kind != business_kind::ucn))
        return unspent;

    for (const auto &row : result)
    {
        if (row.data.get_kind_value() != kind)
            continue;

        if ((row.spend.hash == null_hash) && (status == 0)) // unspent business
            unspent.emplace_back(row);

        if (row.output_height != 0 && (row.spend.hash == null_hash || row.spend_height == 0) && (status == 1)) // confirmed business
            unspent.emplace_back(row);
    }
    return unspent;
}

// get special kind of uid in the database(blockchain)
/*
 status -- // 0 -- unspent  1 -- confirmed
*/
business_history::list address_uid_database::get_business_history(const std::string &address,
                                                                  size_t from_height, business_kind kind, uint32_t time_begin, uint32_t time_end) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    business_history::list unspent;
    // uid type check
    if ((kind != business_kind::uid_register)    // uid_detail
        && (kind != business_kind::uid_transfer) // uid_transfer
        && (kind != business_kind::ucn))
        return unspent;

    for (auto &row : result)
    {
        if (row.data.get_kind_value() != kind || row.data.get_timestamp() < time_begin || row.data.get_timestamp() > time_end)
            continue;

        if ((row.spend.hash == null_hash))
        { //0 -- unspent business
            row.status = 0;
            unspent.emplace_back(row);
        }

        if (row.output_height != 0 && (row.spend.hash == null_hash || row.spend_height == 0))
        { // 1 -- confirmed business
            row.status = 1;
            unspent.emplace_back(row);
        }
    }
    return unspent;
}

// get special kind of uid in the database(blockchain)
business_address_uid::list address_uid_database::get_uids(const std::string &address,
                                                          size_t from_height, business_kind kind) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    business_address_uid::list unspent;
    // uid type check
    if ((kind != business_kind::uid_register)     // uid_detail
        && (kind != business_kind::uid_transfer)) // uid_transfer
        return unspent;

    for (const auto &row : result)
    {
        if (row.data.get_kind_value() != kind)
            continue;

        uint8_t status = 0xff;
        if (row.spend.hash == null_hash)
            status = 0; // 0 -- unspent  1 -- confirmed

        if (row.output_height != 0 &&
            (row.spend.hash == null_hash || row.spend_height == 0))
            status = 1;

        business_address_uid detail;
        if (row.data.get_kind_value() == business_kind::uid_register) // uid register
        {
            auto issue_info = boost::get<uid_detail>(row.data.get_data());
            detail.detail = issue_info;
        }
        else if (row.data.get_kind_value() == business_kind::uid_transfer) //uid transfer
        {
            auto issue_info = boost::get<uid_detail>(row.data.get_data());
            detail.detail = issue_info;
        }

        detail.address = address; // wallet address
        detail.status = status;   // 0 -- unspent  1 -- confirmed
        unspent.emplace_back(detail);
    }
    return unspent;
}

// get all kinds of uid in the database(blockchain)
business_address_uid::list address_uid_database::get_uids(const std::string &address,
                                                          size_t from_height, size_t to_height) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    business_address_uid::list unspent;
    for (const auto &row : result)
    {
        if ((row.data.get_kind_value() != business_kind::uid_register)     // uid_detail
            && (row.data.get_kind_value() != business_kind::uid_transfer)) // uid_transfer
            continue;

        if (address != bc::wallet::payment_address::blackhole_address)
        {
            if (row.output_height > to_height)
            {
                continue;
            }
        }

        uint8_t status = 0xff;
        if (row.spend.hash == null_hash)
            status = 0; // 0 -- unspent  1 -- confirmed

        if (row.output_height != 0 &&
            (row.spend.hash == null_hash || row.spend_height == 0))
            status = 1;

        business_address_uid detail;
        if (row.data.get_kind_value() == business_kind::uid_register) // uid register
        {
            auto issue_info = boost::get<uid_detail>(row.data.get_data());
            detail.detail = issue_info;
        }
        else if (row.data.get_kind_value() == business_kind::uid_transfer) //uid transfer
        {
            auto transfer_info = boost::get<uid_detail>(row.data.get_data());
            detail.detail = transfer_info;
        }

        detail.address = address; // wallet address
        detail.status = status;   // 0 -- unspent  1 -- confirmed
        unspent.emplace_back(detail);
    }
    return unspent;
}

business_address_message::list address_uid_database::get_messages(const std::string &address,
                                                                  size_t from_height) const
{
    data_chunk data(address.begin(), address.end());
    auto key = ripemd160_hash(data);
    business_history::list result = get_business_history(key, from_height);
    business_address_message::list unspent;
    for (const auto &row : result)
    {
        if ((row.data.get_kind_value() != business_kind::message)) // uid_detail
            continue;

        uint8_t status = 0xff;
        if (row.spend.hash == null_hash)
            status = 0; // 0 -- unspent  1 -- confirmed

        if (row.output_height != 0 &&
            (row.spend.hash == null_hash || row.spend_height == 0))
            status = 1;

        business_address_message detail;
        auto issue_info = boost::get<chain::blockchain_message>(row.data.get_data());
        detail.msg = issue_info;

        detail.address = address; // wallet address
        detail.status = status;   // 0 -- unspent  1 -- confirmed
        unspent.emplace_back(detail);
    }
    return unspent;
}
void address_uid_database::sync()
{
    lookup_manager_.sync();
    rows_manager_.sync();
}

address_uid_statinfo address_uid_database::statinfo() const
{
    return {
        lookup_header_.size(),
        lookup_manager_.count(),
        rows_manager_.count()};
}

} // namespace database
} // namespace libbitcoin
