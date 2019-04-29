/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS) 
 *
 * This file is part of uc-node.
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
#include <UChainService/data/databases/token_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/memory/memory.hpp>
//#include <UChain/database/result/token_result.hpp>

namespace libbitcoin
{
namespace database
{

using namespace boost::filesystem;

token_database::token_database(const path &map_filename,
                               std::shared_ptr<shared_mutex> mutex)
    : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
token_database::~token_database()
{
    close();
}

token_result token_database::get_token_result(const hash_digest &hash) const
{
    const auto memory = get(hash);
    return token_result(memory);
}
///
std::shared_ptr<std::vector<token_detail>> token_database::get_token_details() const
{
    auto vec_acc = std::make_shared<std::vector<token_detail>>();
    uint64_t i = 0;
    for (i = 0; i < get_bucket_count(); i++)
    {
        auto memo = lookup_map_.find(i);
        if (memo->size())
        {
            const auto action = [&](memory_ptr elem) {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(token_detail::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

void token_database::store(const hash_digest &hash, const token_detail &sp_detail)
{
    // Write block data.
    const auto key = hash;
    const auto sp_size = sp_detail.serialized_size();
#ifdef UC_DEBUG
    log::debug("token_database::store") << sp_detail.to_string();
#endif
    BITCOIN_ASSERT(sp_size <= max_size_t);
    const auto value_size = static_cast<size_t>(sp_size);

    auto write = [&sp_detail](memory_ptr data) {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(sp_detail.to_data());
    };
    //get_lookup_map().store(key, write, value_size);
    lookup_map_.store(key, write, value_size);
}
} // namespace database
} // namespace libbitcoin
