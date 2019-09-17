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
#include <UChain/coin/message/tx_msg.hpp>

#include <istream>
#include <utility>
#include <UChain/coin/chain/input.hpp>
#include <UChain/coin/chain/output.hpp>
#include <UChain/coin/message/version.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/utility/reader.hpp>

namespace libbitcoin
{
namespace message
{

const std::string tx_message::command = "tx";
const uint32_t tx_message::version_minimum = version::level::minimum;
const uint32_t tx_message::version_maximum = version::level::maximum;

tx_message tx_message::factory_from_data(
    const uint32_t version, const data_chunk &data)
{
    tx_message instance;
    instance.from_data(version, data);
    return instance;
}

tx_message tx_message::factory_from_data(
    const uint32_t version, std::istream &stream)
{
    tx_message instance;
    instance.from_data(version, stream);
    return instance;
}

tx_message tx_message::factory_from_data(
    const uint32_t version, reader &source)
{
    tx_message instance;
    instance.from_data(version, source);
    return instance;
}

tx_message::tx_message()
    : transaction(), originator_(0)
{
}

tx_message::tx_message(const transaction &other)
    : tx_message(other.version, other.locktime, other.inputs,
                          other.outputs)
{
}

tx_message::tx_message(const tx_message &other)
    : tx_message(other.version, other.locktime, other.inputs,
                          other.outputs)
{
}

tx_message::tx_message(uint32_t version, uint32_t locktime,
                                         const chain::input::list &inputs, const chain::output::list &outputs)
    : transaction(version, locktime, inputs, outputs), originator_(0)
{
}

tx_message::tx_message(transaction &&other)
    : tx_message(other.version, other.locktime,
                          std::forward<chain::input::list>(inputs),
                          std::forward<chain::output::list>(outputs))
{
}

tx_message::tx_message(tx_message &&other)
    : tx_message(other.version, other.locktime,
                          std::forward<chain::input::list>(inputs),
                          std::forward<chain::output::list>(outputs))
{
}

tx_message::tx_message(uint32_t version, uint32_t locktime,
                                         chain::input::list &&inputs, chain::output::list &&outputs)
    : transaction(version, locktime, std::forward<chain::input::list>(inputs),
                  std::forward<chain::output::list>(outputs)),
      originator_(0)
{
}

tx_message &tx_message::operator=(
    tx_message &&other)
{
    version = other.version;
    locktime = other.locktime;
    inputs = std::move(other.inputs);
    outputs = std::move(other.outputs);
    originator_ = other.originator_;
    return *this;
}

bool tx_message::from_data(uint32_t version,
                                    const data_chunk &data)
{
    originator_ = version;
    return transaction::from_data(data);
}

bool tx_message::from_data(uint32_t version,
                                    std::istream &stream)
{
    originator_ = version;
    return transaction::from_data(stream);
}

bool tx_message::from_data(uint32_t version, reader &source)
{
    originator_ = version;
    return transaction::from_data(source);
}

data_chunk tx_message::to_data(uint32_t version) const
{
    return transaction::to_data();
}

void tx_message::to_data(uint32_t version,
                                  std::ostream &stream) const
{
    transaction::to_data(stream);
}

void tx_message::to_data(uint32_t version, writer &sink) const
{
    transaction::to_data(sink);
}

uint64_t tx_message::serialized_size(uint32_t version) const
{
    return transaction::serialized_size();
}

uint64_t tx_message::originator() const
{
    return originator_;
}

void tx_message::set_originator(uint64_t value)
{
    originator_ = value;
}

} // namespace message
} // namespace libbitcoin
