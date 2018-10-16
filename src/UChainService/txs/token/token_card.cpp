/**
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#include <UChainService/txs/token/token_card.hpp>
#include <sstream>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChain/blockchain/validate_transaction.hpp>
#include <UChainService/api/restful/utility/Compare.hpp>

namespace libbitcoin {
namespace chain {

// use 1~127 to represent normal mit status type
// add plus 128 to them to make their status type in tracing state.
// status >128 means no content should be store
constexpr uint8_t CARD_STATUS_MASK = 0x7f;
constexpr uint8_t CARD_STATUS_SHORT_OFFSET = 0x80;

token_card::token_card()
{
    reset();
}

token_card::token_card(const std::string& symbol,
                     const std::string& address, const std::string& content)
    : symbol_(symbol)
    , address_(address)
    , content_(content)
    , status_(CARD_STATUS_NONE)
{
}

void token_card::reset()
{
    symbol_ = "";
    address_ = "";
    content_ = "";
    status_ = CARD_STATUS_NONE;
}

bool token_card::is_valid() const
{
    return !(symbol_.empty()
             || address_.empty()
             || is_invalid_status()
             || (calc_size() > get_max_serialized_size()));
}

bool token_card::operator< (const token_card& other) const
{
    return symbol_.compare(other.symbol_) < 0;
}

token_card token_card::factory_from_data(const data_chunk& data)
{
    token_card instance;
    instance.from_data(data);
    return instance;
}

token_card token_card::factory_from_data(std::istream& stream)
{
    token_card instance;
    instance.from_data(stream);
    return instance;
}

token_card token_card::factory_from_data(reader& source)
{
    token_card instance;
    instance.from_data(source);
    return instance;
}

bool token_card::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool token_card::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool token_card::from_data(reader& source)
{
    reset();

    status_ = source.read_byte();
    symbol_ = source.read_string();
    address_ = source.read_string();
    if (is_register_status()) {
        content_ = source.read_string();
    }

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk token_card::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);
    // store status with offset, specify to store no content.
    sink.write_byte(get_status() + CARD_STATUS_SHORT_OFFSET);
    sink.write_string(symbol_);
    sink.write_string(address_);
    ostream.flush();
    return data;
}

data_chunk token_card::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void token_card::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void token_card::to_data(writer& sink) const
{
    sink.write_byte(status_);
    sink.write_string(symbol_);
    sink.write_string(address_);
    if (is_register_status()) {
        sink.write_string(content_);
    }
}

uint64_t token_card::get_max_serialized_size() const
{
    return is_register_status() ? TOKEN_CARD_FIX_SIZE : TOKEN_CARD_TRANSFER_FIX_SIZE;
}

uint64_t token_card::calc_size() const
{
    uint64_t len = (symbol_.size() + 1) + (address_.size() + 1) + TOKEN_CARD_STATUS_FIX_SIZE;
    if (is_register_status()) {
        len += variable_string_size(content_);
    }
    return len;
}

uint64_t token_card::serialized_size() const
{
    return std::min(calc_size(), get_max_serialized_size());
}

std::string token_card::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << get_status_name() << "\n";
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t address = " << address_ << "\n";
    if (is_register_status()) {
        ss << "\t content = " << content_ << "\n";
    }
    return ss.str();
}

const std::string& token_card::get_symbol() const
{
    return symbol_;
}

void token_card::set_symbol(const std::string& symbol)
{
    symbol_ = limit_size_string(symbol, TOKEN_CARD_SYMBOL_FIX_SIZE);
}

const std::string& token_card::get_address() const
{
    return address_;
}

void token_card::set_address(const std::string& address)
{
    address_ = limit_size_string(address, TOKEN_CARD_ADDRESS_FIX_SIZE);
}

const std::string& token_card::get_content() const
{
    return content_;
}

void token_card::set_content(const std::string& content)
{
    content_ = limit_size_string(content, TOKEN_CARD_CONTENT_FIX_SIZE);
}

uint8_t token_card::get_status() const
{
    return status_ & CARD_STATUS_MASK;
}

void token_card::set_status(uint8_t status)
{
    status_ = status & CARD_STATUS_MASK;
}

std::string token_card::status_to_string(uint8_t status)
{
    if (status == CARD_STATUS_REGISTER) {
        return "registered";
    }
    else if (status == CARD_STATUS_TRANSFER) {
        return "transfered";
    }
    else {
        return "none";
    }
}

std::string token_card::get_status_name() const
{
    return status_to_string(get_status());
}

bool token_card::is_register_status() const
{
    return status_ == CARD_STATUS_REGISTER;
}

bool token_card::is_transfer_status() const
{
    return status_ == CARD_STATUS_TRANSFER;
}

bool token_card::is_invalid_status() const
{
    return status_ <= CARD_STATUS_NONE || status_ >= CARD_STATUS_MAX;
}

///////////////////////////////////////////////////
///////////// token_card_info //////////////////////
///////////////////////////////////////////////////
void token_card_info::reset()
{
    output_height = 0;
    timestamp = 0;
    to_uid = "";
    mit.reset();
}

bool token_card_info::operator< (const token_card_info& other) const
{
    return mit < other.mit;
}

uint64_t token_card_info::serialized_size() const
{
    // output_height; timestamp; to_uid; mit;
    return 4 + 4 + (to_uid.size() +1) + mit.serialized_size();
}

token_card_info token_card_info::factory_from_data(reader& source)
{
    token_card_info instance;
    instance.reset();

    instance.output_height = source.read_4_bytes_little_endian();
    instance.timestamp = source.read_4_bytes_little_endian();
    instance.to_uid = source.read_string();
    instance.mit = token_card::factory_from_data(source);

    auto result = static_cast<bool>(source);
    if (!result) {
        instance.reset();
    }

    return instance;
}

data_chunk token_card_info::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_uid);
    sink.write_data(mit.to_data());

    ostream.flush();
    return data;
}

data_chunk token_card_info::to_short_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_4_bytes_little_endian(output_height);
    sink.write_4_bytes_little_endian(timestamp);
    sink.write_string(to_uid);
    sink.write_data(mit.to_short_data());

    ostream.flush();
    return data;
}

} // namspace chain
} // namspace libbitcoin
