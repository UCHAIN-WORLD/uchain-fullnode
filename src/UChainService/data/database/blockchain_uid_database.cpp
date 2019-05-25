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
#include <UChainService/data/databases/blockchain_uid_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/coin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin
{
namespace database
{

using namespace boost::filesystem;

//BC_CONSTEXPR size_t number_buckets = 999997;
BC_CONSTEXPR size_t number_buckets = 9997;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

blockchain_uid_database::blockchain_uid_database(const path &map_filename,
                                                 std::shared_ptr<shared_mutex> mutex)
    : lookup_file_(map_filename, mutex),
      lookup_header_(lookup_file_, number_buckets),
      lookup_manager_(lookup_file_, header_size),
      lookup_map_(lookup_header_, lookup_manager_)
{
}

// Close does not call stop because there is no way to detect thread join.
blockchain_uid_database::~blockchain_uid_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool blockchain_uid_database::create()
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
bool blockchain_uid_database::start()
{
    return lookup_file_.start() &&
           lookup_header_.start() &&
           lookup_manager_.start();
}

// Stop files.
bool blockchain_uid_database::stop()
{
    return lookup_file_.stop();
}

// Close files.
bool blockchain_uid_database::close()
{
    return lookup_file_.close();
}

// ----------------------------------------------------------------------------

void blockchain_uid_database::remove(const hash_digest &hash)
{
    DEBUG_ONLY(bool success =)
    lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void blockchain_uid_database::sync()
{
    lookup_manager_.sync();
}

std::shared_ptr<blockchain_uid> blockchain_uid_database::get(const hash_digest &hash) const
{
    std::shared_ptr<blockchain_uid> detail(nullptr);

    const auto raw_memory = lookup_map_.find(hash);
    if (raw_memory)
    {
        const auto memory = REMAP_ADDRESS(raw_memory);
        detail = std::make_shared<blockchain_uid>();
        auto deserial = make_deserializer_unsafe(memory);
        detail->from_data(deserial);
    }

    return detail;
}

std::shared_ptr<std::vector<blockchain_uid>> blockchain_uid_database::get_history_uids(const hash_digest &hash) const
{
    auto uid_details = std::make_shared<std::vector<blockchain_uid>>();

    const auto raw_memory_vec = lookup_map_.finds(hash);
    for (const auto &raw_memory : raw_memory_vec)
    {
        if (raw_memory)
        {
            const auto memory = REMAP_ADDRESS(raw_memory);
            auto deserial = make_deserializer_unsafe(memory);
            uid_details->emplace_back(blockchain_uid::factory_from_data(deserial));
        }
    }

    return uid_details;
}

///
std::shared_ptr<std::vector<blockchain_uid>> blockchain_uid_database::get_blockchain_uids() const
{
    auto vec_acc = std::make_shared<std::vector<blockchain_uid>>();
    uint64_t i = 0;
    for (i = 0; i < number_buckets; i++)
    {
        auto memo = lookup_map_.find(i);
        //log::debug("get_wallets size=")<<memo->size();
        if (memo->size())
        {
            const auto action = [&](memory_ptr elem) {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(blockchain_uid::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

///
std::shared_ptr<blockchain_uid> blockchain_uid_database::get_register_history(const std::string &uid_symbol) const
{
    std::shared_ptr<blockchain_uid> blockchain_uid_ = nullptr;
    data_chunk data(uid_symbol.begin(), uid_symbol.end());
    auto key = sha256_hash(data);

    auto memo = lookup_map_.rfind(key);
    if (memo)
    {
        blockchain_uid_ = std::make_shared<blockchain_uid>();
        const auto memory = REMAP_ADDRESS(memo);
        auto deserial = make_deserializer_unsafe(memory);
        blockchain_uid_->from_data(deserial);
    }

    return blockchain_uid_;
}

uint64_t blockchain_uid_database::get_register_height(const std::string &uid_symbol) const
{
    std::shared_ptr<blockchain_uid> blockchain_uid_ = get_register_history(uid_symbol);
    if (blockchain_uid_)
        return blockchain_uid_->get_height();

    return max_uint64;
}

void blockchain_uid_database::store(const hash_digest &hash, const blockchain_uid &sp_detail)
{
    // Write block data.
    const auto key = hash;

    //cannot remove old address,instead of update its status
    update_address_status(key, blockchain_uid::address_history);

    const auto sp_size = sp_detail.serialized_size();
#ifdef UC_DEBUG
    log::debug("uid_database::store") << sp_detail.to_string();
#endif
    BITCOIN_ASSERT(sp_size <= max_size_t);
    const auto value_size = static_cast<size_t>(sp_size);

    auto write = [&sp_detail](memory_ptr data) {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(sp_detail.to_data());
    };
    lookup_map_.store(key, write, value_size);
}

std::shared_ptr<blockchain_uid> blockchain_uid_database::update_address_status(const hash_digest &hash, uint32_t status)
{
    std::shared_ptr<blockchain_uid> detail = nullptr;

    const auto raw_memory = lookup_map_.find(hash);
    if (raw_memory)
    {
        detail = std::make_shared<blockchain_uid>();
        if (detail)
        {
            const auto memory = REMAP_ADDRESS(raw_memory);
            auto deserial = make_deserializer_unsafe(memory);
            detail->from_data(deserial);
            if (detail->get_status() != status)
            {
                //update status and serializer
                detail->set_status(status);
                auto serial = make_serializer(memory);
                serial.write_data(detail->to_data());
            }
        }
    }

    return detail;
}

std::shared_ptr<std::vector<blockchain_uid>> blockchain_uid_database::getuids_from_address_history(const std::string &address,
                                                                                                   const uint64_t &fromheight, const uint64_t &toheight) const
{
    auto vec_acc = std::make_shared<std::vector<blockchain_uid>>();
    uint64_t i = 0;
    for (i = 0; i < number_buckets; i++)
    {
        auto sp_memo = lookup_map_.find(i);
        for (auto &elem : *sp_memo)
        {
            const auto memory = REMAP_ADDRESS(elem);
            auto deserial = make_deserializer_unsafe(memory);
            blockchain_uid blockchain_uid_ = blockchain_uid::factory_from_data(deserial);

            const auto height = blockchain_uid_.get_height();
            const auto uid_address = blockchain_uid_.get_uid().get_address();

            if (uid_address == address)
            {
                if ((height >= fromheight && height <= toheight) || (height == max_uint32 && address == bc::wallet::payment_address::blackhole_address))
                {
                    vec_acc->emplace_back(blockchain_uid_);
                }
            }
        }
    }

    std::sort(vec_acc->begin(), vec_acc->end(), [](blockchain_uid &first, blockchain_uid &second) {
        return first.get_height() < second.get_height();
    });
    return vec_acc;
}

std::shared_ptr<blockchain_uid> blockchain_uid_database::pop_uid_transfer(const hash_digest &hash)
{
    lookup_map_.unlink(hash);
    return update_address_status(hash, blockchain_uid::address_current);
}

} // namespace database
} // namespace libbitcoin
