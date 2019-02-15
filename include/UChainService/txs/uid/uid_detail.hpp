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
#ifndef UC_CHAIN_ASSET_UID_DETAIL_HPP
#define UC_CHAIN_ASSET_UID_DETAIL_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t UID_DETAIL_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t UID_DETAIL_ADDRESS_FIX_SIZE = 64;

BC_CONSTEXPR size_t UID_DETAIL_FIX_SIZE = UID_DETAIL_SYMBOL_FIX_SIZE + UID_DETAIL_ADDRESS_FIX_SIZE;

class BC_API uid_detail
{
  public:
    typedef std::vector<uid_detail> list;

    enum uid_detail_type : uint32_t
    {
        created,
        registered_not_in_blockchain,
        registered_in_blockchain
    };

    uid_detail();
    uid_detail(const std::string &symbol, const std::string &address);

    static uid_detail factory_from_data(const data_chunk &data);
    static uid_detail factory_from_data(std::istream &stream);
    static uid_detail factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();
    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;

    bool operator<(const uid_detail &other) const;
    std::string to_string() const;
    void to_json(std::ostream &out);

    bool is_valid() const;
    void reset();
    uint32_t count_size() const;
    uint64_t serialized_size() const;
    const std::string &get_symbol() const;
    void set_symbol(const std::string &symbol);
    const std::string &get_address() const;
    void set_address(const std::string &address);

  private:
    std::string symbol;
    std::string address;
};

} // namespace chain
} // namespace libbitcoin

#endif
