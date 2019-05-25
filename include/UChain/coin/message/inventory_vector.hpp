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
#ifndef UC_MESSAGE_INVENTORY_VECTOR_HPP
#define UC_MESSAGE_INVENTORY_VECTOR_HPP

#include <istream>
#include <UChain/coin/define.hpp>
#include <UChain/coin/math/hash.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API inventory_vector
{
  public:
    typedef std::vector<inventory_vector> list;

    enum class type_id
    {
        error,
        transaction,
        block,
        filtered_block,
        compact_block,
        none
    };

    static type_id to_type(uint32_t value);
    static uint32_t to_number(type_id type);

    static inventory_vector factory_from_data(uint32_t version,
                                              const data_chunk &data);
    static inventory_vector factory_from_data(uint32_t version,
                                              std::istream &stream);
    static inventory_vector factory_from_data(uint32_t version,
                                              reader &source);
    static uint64_t satoshi_fixed_size(uint32_t version);

    bool from_data(uint32_t version, const data_chunk &data);
    bool from_data(uint32_t version, std::istream &stream);
    bool from_data(uint32_t version, reader &source);
    data_chunk to_data(uint32_t version) const;
    void to_data(uint32_t version, std::ostream &stream) const;
    void to_data(uint32_t version, writer &sink) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(uint32_t version) const;
    bool is_block_type() const;
    bool is_transaction_type() const;

    type_id type;
    hash_digest hash;
};

BC_API bool operator==(const inventory_vector &left,
                       const inventory_vector &right);
BC_API bool operator!=(const inventory_vector &left,
                       const inventory_vector &right);

} // namespace message
} // namespace libbitcoin

#endif
