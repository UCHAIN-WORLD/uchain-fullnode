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
#ifndef UC_MESSAGE_REJECT_HPP
#define UC_MESSAGE_REJECT_HPP

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API reject
{
  public:
    enum class error_code : uint8_t
    {
        undefined = 0x00,
        malformed = 0x01,
        invalid = 0x10,
        obsolete = 0x11,
        duplicate = 0x12,
        nonstandard = 0x40,
        dust = 0x41,
        insufficient_fee = 0x42,
        checkpoint = 0x43
    };

    typedef std::shared_ptr<reject> ptr;

    static reject factory_from_data(uint32_t version, const data_chunk &data);
    static reject factory_from_data(uint32_t version, std::istream &stream);
    static reject factory_from_data(uint32_t version, reader &source);

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

    std::string message;
    error_code code;
    std::string reason;
    hash_digest data;

  private:
    static error_code error_code_from_byte(uint8_t byte);
    static uint8_t error_code_to_byte(const error_code code);
};

} // namespace message
} // namespace libbitcoin

#endif
