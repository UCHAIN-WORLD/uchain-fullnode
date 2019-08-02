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
#include <UChain/database/databases/tx_db.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/result/tx_result.hpp>

namespace libbitcoin
{
namespace database
{

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 100000000;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

tx_database::tx_database(const path &map_filename,
                                           std::shared_ptr<shared_mutex> mutex)
    : lookup_file_(map_filename, mutex),
      lookup_header_(lookup_file_, number_buckets),
      lookup_manager_(lookup_file_, header_size),
      lookup_map_(lookup_header_, lookup_manager_)
{
}

// Close does not call stop because there is no way to detect thread join.
tx_database::~tx_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool tx_database::create()
{
    // Resize and create require a started file.
    if (!lookup_file_.start())
        return false;

    // This will throw if insufficient disk space.
    lookup_file_.resize(initial_map_file_size);

    if (!lookup_header_.create() ||
        !lookup_manager_.create())
        return false;

    // Should not call start after create, already started.
    return lookup_header_.start() &&
           lookup_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start files and primitives.
bool tx_database::start()
{
    return lookup_file_.start() &&
           lookup_header_.start() &&
           lookup_manager_.start();
}

// Stop files.
bool tx_database::stop()
{
    return lookup_file_.stop();
}

// Close files.
bool tx_database::close()
{
    return lookup_file_.close();
}

// ----------------------------------------------------------------------------

tx_result tx_database::get(const hash_digest &hash) const
{
    const auto memory = lookup_map_.find(hash);
    return tx_result(memory);
}

void tx_database::store(size_t height, size_t index,
                                 const chain::transaction &tx)
{
    // Write block data.
    const auto key = tx.hash();
    const auto tx_size = tx.serialized_size();

    BITCOIN_ASSERT(height <= max_uint32);
    const auto hight32 = static_cast<size_t>(height);

    BITCOIN_ASSERT(index <= max_uint32);
    const auto index32 = static_cast<size_t>(index);

    BITCOIN_ASSERT(tx_size <= max_size_t - 4 - 4);
    const auto value_size = 4 + 4 + static_cast<size_t>(tx_size);

    auto write = [&hight32, &index32, &tx](memory_ptr data) {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_4_bytes_little_endian(hight32);
        serial.write_4_bytes_little_endian(index32);
        serial.write_data(tx.to_data());
    };
    lookup_map_.store(key, write, value_size);
}

void tx_database::remove(const hash_digest &hash)
{
    DEBUG_ONLY(bool success =)
    lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void tx_database::sync()
{
    lookup_manager_.sync();
}

} // namespace database
} // namespace libbitcoin
