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
#ifndef UC_MESSAGE_HEADERS_HPP
#define UC_MESSAGE_HEADERS_HPP

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <istream>
#include <memory>
#include <string>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/chain/header.hpp>
#include <UChain/bitcoin/math/hash.hpp>
#include <UChain/bitcoin/message/inventory.hpp>
#include <UChain/bitcoin/message/inventory_vector.hpp>
#include <UChain/bitcoin/utility/data.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API headers
{
  public:
    typedef std::shared_ptr<headers> ptr;

    static headers factory_from_data(uint32_t version, const data_chunk &data);
    static headers factory_from_data(uint32_t version, std::istream &stream);
    static headers factory_from_data(uint32_t version, reader &source);

    headers();
    headers(const chain::header::list &values);
    headers(const std::initializer_list<chain::header> &values);

    bool from_data(uint32_t version, const data_chunk &data);
    bool from_data(uint32_t version, std::istream &stream);
    bool from_data(uint32_t version, reader &source);
    data_chunk to_data(uint32_t version) const;
    void to_data(uint32_t version, std::ostream &stream) const;
    void to_data(uint32_t version, writer &sink) const;
    void to_hashes(hash_list &out) const;
    void to_inventory(inventory_vector::list &out,
                      inventory::type_id type) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(uint32_t version) const;

    static const std::string command;
    static const uint32_t version_minimum;
    static const uint32_t version_maximum;

    chain::header::list elements;
};

BC_API bool operator==(const headers &left, const headers &right);
BC_API bool operator!=(const headers &left, const headers &right);

} // namespace message
} // namespace libbitcoin

#endif
