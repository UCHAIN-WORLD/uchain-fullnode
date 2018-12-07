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
#include <UChainService/data/databases/blockchain_card_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 999983;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

blockchain_card_database::blockchain_card_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : lookup_file_(map_filename, mutex),
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size),
    lookup_map_(lookup_header_, lookup_manager_)
{
}

// Close does not call stop because there is no way to detect thread join.
blockchain_card_database::~blockchain_card_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool blockchain_card_database::create()
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
bool blockchain_card_database::start()
{
    return
        lookup_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start();
}

// Stop files.
bool blockchain_card_database::stop()
{
    return lookup_file_.stop();
}

// Close files.
bool blockchain_card_database::close()
{
    return lookup_file_.close();
}

// ----------------------------------------------------------------------------

void blockchain_card_database::remove(const hash_digest& hash)
{
    DEBUG_ONLY(bool success =) lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void blockchain_card_database::sync()
{
    lookup_manager_.sync();
}

std::shared_ptr<token_card_info> blockchain_card_database::get(const hash_digest& hash) const
{
    std::shared_ptr<token_card_info> detail(nullptr);

    const auto raw_memory = lookup_map_.find(hash);
    if(raw_memory) {
        const auto memory = REMAP_ADDRESS(raw_memory);
        detail = std::make_shared<token_card_info>();
        auto deserial = make_deserializer_unsafe(memory);
        *detail = token_card_info::factory_from_data(deserial);
    }

    return detail;
}

std::shared_ptr<token_card_info::list> blockchain_card_database::get_blockchain_cards() const
{
    auto vec_acc = std::make_shared<std::vector<token_card_info>>();
    for( uint64_t i = 0; i < number_buckets; i++ ) {
        auto memo = lookup_map_.find(i);
        if (memo->size()) {
            const auto action = [&vec_acc](memory_ptr elem)
            {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(token_card_info::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

/// 
std::shared_ptr<token_card_info> blockchain_card_database::get_register_history(const std::string & card_symbol) const
{
    std::shared_ptr<token_card_info> token_card_ = nullptr;
    data_chunk data(card_symbol.begin(), card_symbol.end());
    auto key = sha256_hash(data);

    auto memo = lookup_map_.rfind(key);
    if(memo)
    {
        token_card_ = std::make_shared<token_card_info>();
        const auto memory = REMAP_ADDRESS(memo);
        auto deserial = make_deserializer_unsafe(memory);
        *token_card_ = token_card_info::factory_from_data(deserial);
    }

    return token_card_;
}

///
uint64_t blockchain_card_database::get_register_height(const std::string & card_symbol) const
{
    std::shared_ptr<token_card_info> token_card_ = get_register_history(card_symbol);
    if(token_card_)
        return token_card_->output_height;
        
    return max_uint64;
}

void blockchain_card_database::store(const token_card_info& card_info)
{
    const auto& key_str = card_info.mit.get_symbol();
    const data_chunk& data = data_chunk(key_str.begin(), key_str.end());
    const auto key = sha256_hash(data);

#ifdef UC_DEBUG
    log::debug("blockchain_card_database::store") << card_info.mit.to_string();
#endif

    // Write block data.
    const auto sp_size = card_info.serialized_size();
    BITCOIN_ASSERT(sp_size <= max_size_t);
    const auto value_size = static_cast<size_t>(sp_size);

    auto write = [&card_info](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(card_info.to_data());
    };
    lookup_map_.store(key, write, value_size);
}


} // namespace database
} // namespace libbitcoin
