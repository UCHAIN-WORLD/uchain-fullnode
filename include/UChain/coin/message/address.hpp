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
#ifndef UC_MESSAGE_ADDRESS_HPP
#define UC_MESSAGE_ADDRESS_HPP

#include <istream>
#include <memory>
#include <string>
#include <UChain/coin/define.hpp>
#include <UChain/coin/message/network_address.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API address
{
  public:
    typedef std::shared_ptr<address> ptr;

    static address factory_from_data(uint32_t version, const data_chunk &data);
    static address factory_from_data(uint32_t version, std::istream &stream);
    static address factory_from_data(uint32_t version, reader &source);

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

    network_address::list addresses;
};

} // namespace message
} // namespace libbitcoin

#endif
