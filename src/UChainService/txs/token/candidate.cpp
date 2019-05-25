/**
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
#include <UChainService/txs/token/candidate.hpp>
#include <sstream>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChain/blockchain/validate_transaction.hpp>
#include <UChainService/api/restful/utility/Compare.hpp>

namespace libbitcoin
{
namespace chain
{

// use 1~127 to represent normal candidate status type
// add plus 128 to them to make their status type in tracing state.
// status >128 means no content should be store
constexpr uint8_t CANDIDATE_STATUS_MASK = 0x7f;
constexpr uint8_t CANDIDATE_STATUS_SHORT_OFFSET = 0x80;

candidate::candidate()
{
    reset();
}

candidate::candidate(const std::string &symbol,
                     const std::string &address, const std::string &content, uint8_t status)
    : symbol_(symbol), address_(address), content_(content), status_(status)
{
}

void candidate::reset()
{
    symbol_ = "";
    address_ = "";
    content_ = "";
    status_ = CANDIDATE_STATUS_NONE;
}

bool candidate::is_valid() const
{
    return !(symbol_.empty() || address_.empty() || is_invalid_status() || (calc_size() > get_max_serialized_size()));
}

bool candidate::operator<(const candidate &other) const
{
    return symbol_.compare(other.symbol_) < 0;
}

candidate candidate::factory_from_data(const data_chunk &data)
{
    candidate instance;
    instance.from_data(data);
    return instance;
}

candidate candidate::factory_from_data(std::istream &stream)
{
    candidate instance;
    instance.from_data(stream);
    return instance;
}

candidate candidate::factory_from_data(reader &source)
{
    candidate instance;
    instance.from_data(source);
    return instance;
}

bool candidate::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool candidate::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool candidate::from_data(reader &source)
{
    reset();

    status_ = source.read_byte();
    symbol_ = source.read_string();
    address_ = source.read_string();
    if (is_register_status())
    {
        content_ = source.read_string();
    }

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk candidate::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);
    // store status with offset, specify to store no content.
    sink.write_byte(get_status() + CANDIDATE_STATUS_SHORT_OFFSET);
    sink.write_string(symbol_);
    sink.write_string(address_);
    ostream.flush();
    return data;
}

data_chunk candidate::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void candidate::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void candidate::to_data(writer &sink) const
{
    sink.write_byte(status_);
    sink.write_string(symbol_);
    sink.write_string(address_);
    if (is_register_status())
    {
        sink.write_string(content_);
    }
}

uint64_t candidate::get_max_serialized_size() const
{
    return is_register_status() ? TOKEN_CANDIDATE_FIX_SIZE : TOKEN_CANDIDATE_TRANSFER_FIX_SIZE;
}

uint64_t candidate::calc_size() const
{
    uint64_t len = (symbol_.size() + 1) + (address_.size() + 1) + TOKEN_CANDIDATE_STATUS_FIX_SIZE;
    if (is_register_status())
    {
        len += variable_string_size(content_);
    }
    return len;
}

uint64_t candidate::serialized_size() const
{
    return std::min(calc_size(), get_max_serialized_size());
}

std::string candidate::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << get_status_name() << "\n";
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t address = " << address_ << "\n";
    if (is_register_status())
    {
        ss << "\t content = " << content_ << "\n";
    }
    return ss.str();
}

const std::string &candidate::get_symbol() const
{
    return symbol_;
}

void candidate::set_symbol(const std::string &symbol)
{
    symbol_ = limit_size_string(symbol, TOKEN_CANDIDATE_SYMBOL_FIX_SIZE);
}

const std::string &candidate::get_address() const
{
    return address_;
}

void candidate::set_address(const std::string &address)
{
    address_ = limit_size_string(address, TOKEN_CANDIDATE_ADDRESS_FIX_SIZE);
}

const std::string &candidate::get_content() const
{
    return content_;
}

void candidate::set_content(const std::string &content)
{
    content_ = limit_size_string(content, TOKEN_CANDIDATE_CONTENT_FIX_SIZE);
}

uint8_t candidate::get_status() const
{
    return status_ & CANDIDATE_STATUS_MASK;
}

void candidate::set_status(uint8_t status)
{
    status_ = status & CANDIDATE_STATUS_MASK;
}

std::string candidate::status_to_string(uint8_t status)
{
    if (status == CANDIDATE_STATUS_REGISTER)
    {
        return "registered";
    }
    else if (status == CANDIDATE_STATUS_TRANSFER)
    {
        return "transfered";
    }
    else if (status == CANDIDATE_STATUS_HISTORY)
    {
        return "history";
    }
    else if (status == CANDIDATE_STATUS_CURRENT)
    {
        return "current";
    }
    else
    {
        return "none";
    }
}

std::string candidate::get_status_name() const
{
    return status_to_string(get_status());
}

bool candidate::is_register_status() const
{
    return status_ == CANDIDATE_STATUS_REGISTER;
}

bool candidate::is_transfer_status() const
{
    return status_ == CANDIDATE_STATUS_TRANSFER;
}

bool candidate::is_invalid_status() const
{
    return status_ <= CANDIDATE_STATUS_NONE || status_ >= CANDIDATE_STATUS_MAX;
}

///////////////////////////////////////////////////
///////////// candidate_info //////////////////////
///////////////////////////////////////////////////
void candidate_info::reset()
{
    output_height = 0;
    timestamp = 0;
    vote = 0;
    //status = 0;
    to_uid = "";
    candidate.reset();
}

bool candidate_info::operator<(const candidate_info &other) const
{
    return candidate < other.candidate;
}

uint64_t candidate_info::serialized_size() const
{
    // output_height; timestamp; to_uid; candidate;
    return 4 + 4 + (to_uid.size() + 1) + candidate.serialized_size();
}

candidate_info candidate_info::factory_from_data(reader &source)
{
    candidate_info instance;
    instance.reset();

    instance.output_height = source.read_4_bytes_little_endian();
    instance.timestamp = source.read_4_bytes_little_endian();
    //instance.status = source.read_byte();
    instance.to_uid = source.read_string();
    instance.candidate = candidate::factory_from_data(source);

    auto result = static_cast<bool>(source);
    if (!result)
    {
        instance.reset();
    }

    return instance;
}

data_chunk candidate_info::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_uid);
    sink.write_data(candidate.to_data());

    ostream.flush();
    return data;
}

data_chunk candidate_info::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_uid);
    sink.write_data(candidate.to_short_data());

    ostream.flush();
    return data;
}

// const uint8_t& candidate_info::get_status()
// {
//     return this->status;
// }
// void candidate_info::set_status(const uint8_t &status)
// {
//     this->status = status;
// }

} // namespace chain
} // namespace libbitcoin