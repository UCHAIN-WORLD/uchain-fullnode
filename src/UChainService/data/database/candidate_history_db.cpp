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
#include <UChainService/data/databases/candidate_history_db.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/primitives/record_multimap_iterable.hpp>
#include <UChain/database/primitives/record_multimap_iterator.hpp>

#define LOG_CANDIDATE_HISTORY_DATABASE "candidate_history_database"

namespace libbitcoin
{
namespace database
{

using namespace boost::filesystem;
using namespace bc::chain;

namespace
{
// Read the height value from the row.
uint32_t read_height(uint8_t *data)
{
    return from_little_endian_unsafe<uint32_t>(data);
};

// Read the height value from the row.
uint32_t read_time(uint8_t *data)
{
    return from_little_endian_unsafe<uint32_t>(data + 4);
};

// Read a row from the data for the history list.
candidate_info read_row(uint8_t *data)
{
    auto deserial = make_deserializer_unsafe(data);
    return candidate_info::factory_from_data(deserial);
};
} // namespace

BC_CONSTEXPR size_t number_buckets = 99999989;
BC_CONSTEXPR size_t header_size = record_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_lookup_file_size = header_size + minimum_records_size;

BC_CONSTEXPR size_t record_size = hash_table_multimap_record_size<short_hash>();

BC_CONSTEXPR size_t candidate_transfer_record_size = TOKEN_CANDIDATE_INFO_FIX_SIZE;
BC_CONSTEXPR size_t row_record_size = hash_table_record_size<hash_digest>(candidate_transfer_record_size);

candidate_history_database::candidate_history_database(const path &lookup_filename,
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
candidate_history_database::~candidate_history_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool candidate_history_database::create()
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

bool candidate_history_database::start()
{
    return lookup_file_.start() &&
           rows_file_.start() &&
           lookup_header_.start() &&
           lookup_manager_.start() &&
           rows_manager_.start();
}

bool candidate_history_database::stop()
{
    return lookup_file_.stop() &&
           rows_file_.stop();
}

bool candidate_history_database::close()
{
    return lookup_file_.close() &&
           rows_file_.close();
}

void candidate_history_database::sync()
{
    lookup_manager_.sync();
    rows_manager_.sync();
}

candidate_history_statinfo candidate_history_database::statinfo() const
{
    return {
        lookup_header_.size(),
        lookup_manager_.count(),
        rows_manager_.count()};
}

// ----------------------------------------------------------------------------
void candidate_history_database::store(const candidate_info &candidate_info)
{
    const auto &key_str = candidate_info.candidate.get_symbol();
    const data_chunk &data = data_chunk(key_str.begin(), key_str.end());
    const auto key = ripemd160_hash(data);

    auto write = [&candidate_info](memory_ptr data) {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(candidate_info.to_short_data());
    };
    rows_multimap_.add_row(key, write);
}

void candidate_history_database::delete_last_row(const short_hash &key)
{
    rows_multimap_.delete_last_row(key);
}

std::shared_ptr<candidate_info> candidate_history_database::get(const short_hash &key) const
{
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    for (const auto index : records)
    {
        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        return std::make_shared<candidate_info>(read_row(address));
    }

    return nullptr;
}

std::shared_ptr<candidate_info::list> candidate_history_database::get_history_candidates_by_height(
    const short_hash &key, uint32_t start_height, uint32_t end_height,
    uint64_t limit, uint64_t page_number) const
{
    // use map to sort by height, decreasely
    std::map<uint32_t, candidate_info, std::greater<uint32_t>> height_candidate_map;

    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    uint64_t cnt = 0;
    for (const auto index : records)
    {
        // Stop once we reach the limit (if specified).
        if ((limit > 0) && (height_candidate_map.size() >= limit))
        {
            break;
        }

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        const auto height = read_height(address);

        // Stop once we reach the end (if specified).
        if ((end_height > 0) && (height > end_height))
        {
            break;
        }

        // Skip rows below from_height.
        if (height < start_height)
        {
            continue;
        }

        cnt++;
        if ((limit > 0) && (page_number > 0) && ((cnt - 1) / limit) < (page_number - 1))
        {
            continue; // skip previous page record
        }

        auto row = read_row(address);
        height_candidate_map[height] = row;
    }

    auto result = std::make_shared<candidate_info::list>();
    for (const auto &pair : height_candidate_map)
    {
        result->emplace_back(std::move(pair.second));
    }
    return result;
}

std::shared_ptr<candidate_info::list> candidate_history_database::get_history_candidates_by_time(
    const short_hash &key, uint32_t time_begin, uint32_t time_end,
    uint64_t limit, uint64_t page_number) const
{
    // use map to sort by time, decreasely
    std::map<uint32_t, candidate_info, std::greater<uint32_t>> time_candidate_map;

    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    uint64_t cnt = 0;
    for (const auto index : records)
    {
        // Stop once we reach the limit (if specified).
        if ((limit > 0) && (time_candidate_map.size() >= limit))
        {
            break;
        }

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        const auto time = read_time(address);

        // Stop once we reach the end (if specified).
        if ((time_end > 0) && (time > time_end))
        {
            break;
        }

        // Skip rows below from_height.
        if (time < time_begin)
        {
            continue;
        }

        cnt++;
        if ((limit > 0) && (page_number > 0) && ((cnt - 1) / limit) < (page_number - 1))
        {
            continue; // skip previous page record
        }

        auto row = read_row(address);
        time_candidate_map[time] = row;
    }

    auto result = std::make_shared<candidate_info::list>();
    for (const auto &pair : time_candidate_map)
    {
        result->emplace_back(std::move(pair.second));
    }
    return result;
}

bool candidate_history_database::update_address_status(const candidate_info &candidate_info, uint8_t status)
{
    const auto &key_str = candidate_info.candidate.get_symbol();
    const data_chunk &data = data_chunk(key_str.begin(), key_str.end());
    const auto key = ripemd160_hash(data);

    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);
    const auto record = rows_list_.get(start);

    if (record)
    {
        const auto address = REMAP_ADDRESS(record);
        auto row = read_row(address);
        if (row.candidate.get_status() != status)
        {
            //update status and serializer
            row.candidate.set_status(status);
            auto serial = make_serializer(address);
            serial.write_data(row.to_data());
        }
        return true;
    }

    return false;
}

} // namespace database
} // namespace libbitcoin
