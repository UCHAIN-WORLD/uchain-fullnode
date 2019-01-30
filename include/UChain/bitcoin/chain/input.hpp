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
#ifndef UC_CHAIN_INPUT_HPP
#define UC_CHAIN_INPUT_HPP

#include <cstdint>
#include <istream>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/math/hash.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <UChain/bitcoin/wallet/payment_address.hpp>

namespace libbitcoin
{
namespace chain
{

class BC_API input
{
  public:
    typedef std::vector<input> list;

    static input factory_from_data(const data_chunk &data);
    static input factory_from_data(std::istream &stream);
    static input factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    void reset();
    bool is_final() const;
    uint64_t serialized_size() const;
    std::string get_script_address() const;

    output_point previous_output;
    chain::script script;
    uint32_t sequence;
};

} // namespace chain
} // namespace libbitcoin

#endif
