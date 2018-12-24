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
#include <UChainService/data/databases/wallet_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

namespace {
    // copy from block_chain_imp.cpp
    hash_digest get_hash(const std::string& str)
    {
        data_chunk data(str.begin(), str.end());
        return sha256_hash(data);
    }
}

wallet_database::wallet_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
wallet_database::~wallet_database()
{
    close();
}

void wallet_database::set_admin(const std::string& name, const std::string& passwd)
{
    // create admin wallet if not exists
    const auto hash = get_hash(name);
    if( nullptr == get(hash)) {
        libbitcoin::chain::wallet acc;
        acc.set_name(name);
        acc.set_passwd(passwd);
        acc.set_priority(wallet_priority::administrator);
        store(acc);
        sync();
    }
}

void wallet_database::store(const libbitcoin::chain::wallet& wallet)
{
    const auto& name = wallet.get_name();
    const auto hash = get_hash(name);
    const auto wallet_data = wallet.to_data();

    const auto check_store = [this, &name, &hash, &wallet_data]() {
        auto&& result = get_wallet_result(hash);
        if (!result) {
            return true;
        }

        auto detail = result.get_wallet_detail();
        if (!detail) {
            return true;
        }

        // wallet exist -- check duplicate
        if (detail->to_data() == wallet_data) {
            // if completely same, no need to store the same data.
            return false;
        }

        // wallet exist -- check hash conflict
        if (detail->get_name() != name) {
            log::error("wallet_database")
                << detail->get_name()
                << " is already exist and has same hash "
                << encode_hash(hash) << " with this name "
                << name;
            // for security reason, don't store wallet with hash conflict.
            return false;
        }

        // wallet exist -- remove old value
        remove(hash);
        sync();
        return true;
    };

    if (check_store()) {
        // actually store wallet
        const auto acc_size = wallet.serialized_size();
        BITCOIN_ASSERT(acc_size <= max_size_t);
        const auto value_size = static_cast<size_t>(acc_size);

        auto write = [&wallet_data](memory_ptr data)
        {
            auto serial = make_serializer(REMAP_ADDRESS(data));
            serial.write_data(wallet_data);
        };

        lookup_map_.store(hash, write, value_size);
    }
}

std::shared_ptr<std::vector<libbitcoin::chain::wallet>> wallet_database::get_wallets() const
{
    auto vec_acc = std::make_shared<std::vector<libbitcoin::chain::wallet>>();
    for (size_t i = 0; i < get_bucket_count(); i++ ) {
        auto memo = lookup_map_.find(i);
        if (memo->size()) {
            const auto action = [&vec_acc](memory_ptr elem)
            {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(libbitcoin::chain::wallet::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

wallet_result wallet_database::get_wallet_result(const hash_digest& hash) const
{
    const auto memory = get(hash);
    return wallet_result(memory);
}

} // namespace database
} // namespace libbitcoin
