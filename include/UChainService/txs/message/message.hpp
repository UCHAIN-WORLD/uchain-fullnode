/**
 * Copyright (c) 2018-2020 UChain developers 
 *
 * This file is part of UChainService.
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
#ifndef UC_CHAIN_ATTACH_blockchain_message_HPP
#define UC_CHAIN_ATTACH_blockchain_message_HPP

#include <cstdint>
#include <istream>
//#include <UChain/coin/chain/point.hpp>
//#include <UChain/coin/chain/script/script.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t BLOCKCHAIN_MESSAGE_FIX_SIZE = 256;
class BC_API blockchain_message
{
  public:
    //BC_CONSTEXPR static size_t blockchain_message_FIX_SIZE = 64;

    blockchain_message();
    blockchain_message(std::string content);
    static blockchain_message factory_from_data(const data_chunk &data);
    static blockchain_message factory_from_data(std::istream &stream);
    static blockchain_message factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    std::string to_string() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const std::string &get_content() const;
    void set_content(const std::string &content);

  private:
    std::string content_;
};

} // namespace chain
} // namespace libbitcoin

#endif
