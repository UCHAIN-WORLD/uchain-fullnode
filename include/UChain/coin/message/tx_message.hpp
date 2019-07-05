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
#ifndef UC_MESSAGE_tx_message_HPP
#define UC_MESSAGE_tx_message_HPP

#include <cstdint>
#include <cstddef>
#include <istream>
#include <memory>
#include <UChain/coin/define.hpp>
#include <UChain/coin/chain/input.hpp>
#include <UChain/coin/chain/output.hpp>
#include <UChain/coin/chain/transaction.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/reader.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API tx_message
    : public chain::transaction
{
  public:
    typedef std::vector<tx_message> list;
    typedef std::shared_ptr<tx_message> ptr;
    typedef std::vector<ptr> ptr_list;
    typedef std::vector<size_t> indexes;

    static tx_message factory_from_data(uint32_t version,
                                                 const data_chunk &data);
    static tx_message factory_from_data(uint32_t version,
                                                 std::istream &stream);
    static tx_message factory_from_data(uint32_t version,
                                                 reader &source);

    tx_message();
    tx_message(const transaction &other);
    tx_message(const tx_message &other);
    tx_message(uint32_t version, uint32_t locktime,
                        const chain::input::list &inputs, const chain::output::list &outputs);

    tx_message(transaction &&other);
    tx_message(tx_message &&other);
    tx_message(uint32_t version, uint32_t locktime,
                        chain::input::list &&inputs, chain::output::list &&outputs);

    /// This class is move assignable but not copy assignable.
    tx_message &operator=(tx_message &&other);
    void operator=(const tx_message &) = delete;

    bool from_data(uint32_t version, const data_chunk &data);
    bool from_data(uint32_t version, std::istream &stream);
    bool from_data(uint32_t version, reader &source);
    data_chunk to_data(uint32_t version) const;
    void to_data(uint32_t version, std::ostream &stream) const;
    void to_data(uint32_t version, writer &sink) const;
    uint64_t serialized_size(uint32_t version) const;
    uint64_t originator() const;
    void set_originator(uint64_t value);

    static const std::string command;
    static const uint32_t version_minimum;
    static const uint32_t version_maximum;

  private:
    uint64_t originator_;
};

} // namespace message
} // namespace libbitcoin

#endif
