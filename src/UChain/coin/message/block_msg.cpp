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
#include <UChain/coin/message/block_msg.hpp>

#include <cstdint>
#include <cstddef>
#include <istream>
#include <utility>
#include <UChain/coin/message/version.hpp>
#include <UChain/coin/chain/header.hpp>
#include <UChain/coin/chain/transaction.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/reader.hpp>

namespace libbitcoin
{
namespace message
{

const std::string block_msg::command = "block";
const uint32_t block_msg::version_minimum = version::level::minimum;
const uint32_t block_msg::version_maximum = version::level::maximum;

block_msg block_msg::factory_from_data(uint32_t version,
                                               const data_chunk &data, bool with_transaction_count)
{
    block_msg instance;
    instance.from_data(version, data, with_transaction_count);
    return instance;
}

block_msg block_msg::factory_from_data(uint32_t version,
                                               std::istream &stream, bool with_transaction_count)
{
    block_msg instance;
    instance.from_data(version, stream, with_transaction_count);
    return instance;
}

block_msg block_msg::factory_from_data(uint32_t version,
                                               reader &source, bool with_transaction_count)
{
    block_msg instance;
    instance.from_data(version, source, with_transaction_count);
    return instance;
}

block_msg::block_msg()
    : block(), originator_(0)
{
}

block_msg::block_msg(const block &other)
    : block(other)
{
}

block_msg::block_msg(const block_msg &other)
    : block_msg(other.header, other.transactions)
{
}

block_msg::block_msg(const chain::header &header,
                             const chain::transaction::list &transactions)
    : block(header, transactions), originator_(0)
{
}

block_msg::block_msg(block &&other)
    : block(std::forward<block>(other))
{
}

block_msg::block_msg(block_msg &&other)
    : block_msg(std::forward<chain::header>(other.header),
                    std::forward<chain::transaction::list>(other.transactions))
{
}

block_msg::block_msg(chain::header &&header,
                             chain::transaction::list &&transactions)
    : block(std::forward<chain::header>(header),
            std::forward<chain::transaction::list>(transactions)),
      originator_(0)
{
}

block_msg &block_msg::operator=(block_msg &&other)
{
    header = std::move(other.header);
    transactions = std::move(other.transactions);
    originator_ = other.originator_;
    return *this;
}

bool block_msg::from_data(uint32_t version, const data_chunk &data,
                              bool with_transaction_count)
{
    originator_ = version;
    return block::from_data(data, with_transaction_count);
}

bool block_msg::from_data(uint32_t version, std::istream &stream,
                              bool with_transaction_count)
{
    originator_ = version;
    return block::from_data(stream, with_transaction_count);
}

bool block_msg::from_data(uint32_t version, reader &source,
                              bool with_transaction_count)
{
    originator_ = version;
    return block::from_data(source, with_transaction_count);
}

data_chunk block_msg::to_data(uint32_t version,
                                  bool with_transaction_count) const
{
    return block::to_data(with_transaction_count);
}

void block_msg::to_data(uint32_t version, std::ostream &stream,
                            bool with_transaction_count) const
{
    block::to_data(stream, with_transaction_count);
}

void block_msg::to_data(uint32_t version, writer &sink,
                            bool with_transaction_count) const
{
    block::to_data(sink, with_transaction_count);
}

uint64_t block_msg::serialized_size(uint32_t version,
                                        bool with_transaction_count) const
{
    return block::serialized_size(with_transaction_count);
}

uint64_t block_msg::originator() const
{
    return originator_;
}

void block_msg::set_originator(uint64_t value)
{
    originator_ = value;
}

} // namespace message
} // namespace libbitcoin
