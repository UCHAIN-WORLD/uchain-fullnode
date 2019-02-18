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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChain/bitcoin/message/not_found.hpp>

#include <initializer_list>
#include <UChain/bitcoin/math/hash.hpp>
#include <UChain/bitcoin/message/inventory.hpp>
#include <UChain/bitcoin/message/version.hpp>

namespace libbitcoin
{
namespace message
{

const std::string not_found::command = "notfound";
const uint32_t not_found::version_minimum = version::level::bip37;
const uint32_t not_found::version_maximum = version::level::maximum;

not_found not_found::factory_from_data(uint32_t version,
                                       const data_chunk &data)
{
    not_found instance;
    instance.from_data(version, data);
    return instance;
}

not_found not_found::factory_from_data(uint32_t version,
                                       std::istream &stream)
{
    not_found instance;
    instance.from_data(version, stream);
    return instance;
}

not_found not_found::factory_from_data(uint32_t version,
                                       reader &source)
{
    not_found instance;
    instance.from_data(version, source);
    return instance;
}

not_found::not_found()
    : inventory()
{
}

not_found::not_found(const inventory_vector::list &values)
    : inventory(values)
{
}

not_found::not_found(const hash_list &hashes, inventory::type_id type)
    : inventory(hashes, type)
{
}

not_found::not_found(const std::initializer_list<inventory_vector> &values)
    : inventory(values)
{
}

bool not_found::from_data(uint32_t version, const data_chunk &data)
{
    return inventory::from_data(version, data);
}

bool not_found::from_data(uint32_t version, std::istream &stream)
{
    return inventory::from_data(version, stream);
}

bool not_found::from_data(uint32_t version, reader &source)
{
    bool result = !(version < not_found::version_minimum);

    if (result)
        result = inventory::from_data(version, source);

    if (!result)
        reset();

    return result;
}

} // namespace message
} // namespace libbitcoin
