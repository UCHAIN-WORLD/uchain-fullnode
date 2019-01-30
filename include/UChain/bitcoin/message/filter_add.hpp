/*
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
#ifndef UC_MESSAGE_FILTER_ADD_HPP
#define UC_MESSAGE_FILTER_ADD_HPP

#include <istream>
#include <memory>
#include <string>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/data.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API filter_add
{
  public:
    typedef std::shared_ptr<filter_add> ptr;

    static filter_add factory_from_data(uint32_t version,
                                        const data_chunk &data);
    static filter_add factory_from_data(uint32_t version,
                                        std::istream &stream);
    static filter_add factory_from_data(uint32_t version, reader &source);

    bool from_data(uint32_t version, const data_chunk &data);
    bool from_data(uint32_t version, std::istream &stream);
    bool from_data(uint32_t version, reader &source);
    data_chunk to_data(uint32_t version) const;
    void to_data(uint32_t version, std::ostream &stream) const;
    void to_data(uint32_t version, writer &sink) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(uint32_t version) const;

    static const std::string command;
    static const uint32_t version_minimum;
    static const uint32_t version_maximum;

    data_chunk data;
};

BC_API bool operator==(const filter_add &left, const filter_add &right);
BC_API bool operator!=(const filter_add &left, const filter_add &right);

} // namespace message
} // namespace libbitcoin

#endif
