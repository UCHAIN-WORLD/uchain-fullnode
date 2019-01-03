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
#include <UChainService/data/databases/blockchain_candidate_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 9997; //999983;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

blockchain_candidate_database::blockchain_candidate_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : lookup_file_(map_filename, mutex),
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size),
    lookup_map_(lookup_header_, lookup_manager_)
{
}

// Close does not call stop because there is no way to detect thread join.
blockchain_candidate_database::~blockchain_candidate_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool blockchain_candidate_database::create()
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
    return
        lookup_header_.start() &&
        lookup_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start files and primitives.
bool blockchain_candidate_database::start()
{
    return
        lookup_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start();
}

// Stop files.
bool blockchain_candidate_database::stop()
{
    return lookup_file_.stop();
}

// Close files.
bool blockchain_candidate_database::close()
{
    return lookup_file_.close();
}

// ----------------------------------------------------------------------------

void blockchain_candidate_database::remove(const hash_digest& hash)
{
    DEBUG_ONLY(bool success =) lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void blockchain_candidate_database::sync()
{
    lookup_manager_.sync();
}

std::shared_ptr<candidate_info> blockchain_candidate_database::get(const hash_digest& hash) const
{
    std::shared_ptr<candidate_info> detail(nullptr);

    const auto raw_memory = lookup_map_.find(hash);
    if(raw_memory) {
        const auto memory = REMAP_ADDRESS(raw_memory);
        detail = std::make_shared<candidate_info>();
        auto deserial = make_deserializer_unsafe(memory);
        *detail = candidate_info::factory_from_data(deserial);
    }

    return detail;
}

std::shared_ptr<candidate_info::list> blockchain_candidate_database::get_blockchain_candidates() const
{
    auto vec_acc = std::make_shared<std::vector<candidate_info>>();
    for( uint64_t i = 0; i < number_buckets; i++ ) {
        auto memo = lookup_map_.find(i);
        if (memo->size()) {
            const auto action = [&vec_acc](memory_ptr elem)
            {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(candidate_info::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

/// 
std::shared_ptr<candidate_info> blockchain_candidate_database::get_register_history(const std::string & candidate_symbol) const
{
    std::shared_ptr<candidate_info> candidate_ = nullptr;
    data_chunk data(candidate_symbol.begin(), candidate_symbol.end());
    auto key = sha256_hash(data);

    auto memo = lookup_map_.rfind(key);
    if(memo)
    {
        candidate_ = std::make_shared<candidate_info>();
        const auto memory = REMAP_ADDRESS(memo);
        auto deserial = make_deserializer_unsafe(memory);
        *candidate_ = candidate_info::factory_from_data(deserial);
    }

    return candidate_;
}

///
uint64_t blockchain_candidate_database::get_register_height(const std::string & candidate_symbol) const
{
    std::shared_ptr<candidate_info> candidate_ = get_register_history(candidate_symbol);
    if(candidate_)
        return candidate_->output_height;
        
    return max_uint64;
}

void blockchain_candidate_database::store(candidate_info& candidate_info)
{
    const auto& key_str = candidate_info.candidate.get_symbol();
    const data_chunk& data = data_chunk(key_str.begin(), key_str.end());
    const auto key = sha256_hash(data);
    if(lookup_map_.find(key))
        remove(key);
#ifdef UC_DEBUG
    log::debug("blockchain_candidate_database::store") << candidate_info.candidate.to_string();
#endif

    // Write block data.
    const auto sp_size = candidate_info.serialized_size();
    BITCOIN_ASSERT(sp_size <= max_size_t);
    const auto value_size = static_cast<size_t>(sp_size);

    auto write = [&candidate_info](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(candidate_info.to_data());
    };
    lookup_map_.store(key, write, value_size);
}


} // namespace database
} // namespace libbitcoin
