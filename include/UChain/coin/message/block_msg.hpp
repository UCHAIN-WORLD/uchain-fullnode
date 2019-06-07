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
#ifndef UC_MESSAGE_block_msg_HPP
#define UC_MESSAGE_block_msg_HPP

#include <cstdint>
#include <cstddef>
#include <istream>
#include <memory>
#include <UChain/coin/chain/block.hpp>
#include <UChain/coin/chain/header.hpp>
#include <UChain/coin/chain/transaction.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/reader.hpp>

namespace libbitcoin
{
namespace message
{

class BC_API block_msg
    : public chain::block
{
  public:
    typedef std::vector<block_msg> list;
    typedef std::shared_ptr<block_msg> ptr;
    typedef std::vector<ptr> ptr_list;
    typedef std::vector<size_t> indexes;

    static block_msg factory_from_data(uint32_t version,
                                           const data_chunk &data, bool with_transaction_count = true);
    static block_msg factory_from_data(uint32_t version,
                                           std::istream &stream, bool with_transaction_count = true);
    static block_msg factory_from_data(uint32_t version,
                                           reader &source, bool with_transaction_count = true);

    block_msg();
    block_msg(const chain::block &other);
    block_msg(const block_msg &other);
    block_msg(const chain::header &header,
                  const chain::transaction::list &transactions);

    block_msg(chain::block &&other);
    block_msg(block_msg &&other);
    block_msg(chain::header &&header,
                  chain::transaction::list &&transactions);

    /// This class is move assignable but not copy assignable.
    block_msg &operator=(block_msg &&other);
    void operator=(const block_msg &) = delete;

    bool from_data(uint32_t version, const data_chunk &data,
                   bool with_transaction_count = true);
    bool from_data(uint32_t version, std::istream &stream,
                   bool with_transaction_count = true);
    bool from_data(uint32_t version, reader &source,
                   bool with_transaction_count = true);
    data_chunk to_data(uint32_t version,
                       bool with_transaction_count = true) const;
    void to_data(uint32_t version, std::ostream &stream,
                 bool with_transaction_count = true) const;
    void to_data(uint32_t version, writer &sink,
                 bool with_transaction_count = true) const;
    uint64_t serialized_size(uint32_t version,
                             bool with_transaction_count = true) const;

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
