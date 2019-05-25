/**
 * Copyright (c) 2018-2020 UChain developers 
 *
 * This file is part of uc-node.
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
#include <UChainService/txs/token/token_detail.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChain/coin/utility/string.hpp>
#include <json/minijson_writer.hpp>

#define TOKEN_SYMBOL_DELIMITER "."

namespace libbitcoin
{
namespace chain
{

token_detail::token_detail()
{
    reset();
}

token_detail::token_detail(
    const std::string &symbol, uint64_t maximum_supply,
    uint8_t decimal_number, uint8_t threshold, const std::string &issuer,
    const std::string &address, const std::string &description) : symbol(symbol), maximum_supply(maximum_supply),
                                                                  decimal_number(decimal_number),
                                                                  secondaryissue_threshold(threshold),
                                                                  unused2(0), unused3(0),
                                                                  issuer(issuer), address(address), description(description)
{
}

token_detail token_detail::factory_from_data(const data_chunk &data)
{
    token_detail instance;
    instance.from_data(data);
    return instance;
}

token_detail token_detail::factory_from_data(std::istream &stream)
{
    token_detail instance;
    instance.from_data(stream);
    return instance;
}

token_detail token_detail::factory_from_data(reader &source)
{
    token_detail instance;
    instance.from_data(source);
    return instance;
}

bool token_detail::is_valid() const
{
    return !(symbol.empty() || (maximum_supply == 0 && !is_token_secondaryissue()) || (symbol.size() + 8 + 4 + issuer.size() + address.size() + description.size() + 4) > TOKEN_DETAIL_FIX_SIZE);
}

void token_detail::reset()
{
    symbol = "";
    maximum_supply = 0;
    decimal_number = 0;
    secondaryissue_threshold = 0;
    unused2 = 0;
    unused3 = 0;
    issuer = "";
    address = "";
    description = "";
}

bool token_detail::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool token_detail::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool token_detail::from_data(reader &source)
{
    reset();

    symbol = source.read_string();
    maximum_supply = source.read_8_bytes_little_endian();
    decimal_number = source.read_byte();
    secondaryissue_threshold = source.read_byte();
    unused2 = source.read_byte();
    unused3 = source.read_byte();
    issuer = source.read_string();
    address = source.read_string();
    description = source.read_string();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk token_detail::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void token_detail::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void token_detail::to_data(writer &sink) const
{
    sink.write_string(symbol);
    sink.write_8_bytes_little_endian(maximum_supply);
    sink.write_byte(decimal_number);
    sink.write_byte(secondaryissue_threshold);
    sink.write_byte(unused2);
    sink.write_byte(unused3);
    sink.write_string(issuer);
    sink.write_string(address);
    sink.write_string(description);
}

uint64_t token_detail::serialized_size() const
{
    size_t len = symbol.size() + 8 + 4 + issuer.size() + address.size() + description.size() + 4;
    return std::min(TOKEN_DETAIL_FIX_SIZE, len);
}

std::string token_detail::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
       << "\t maximum_supply = " << std::to_string(maximum_supply) << "\n"
       << "\t decimal_number = " << std::to_string(decimal_number) << "\n"
       << "\t is_token_secondaryissue = " << (is_token_secondaryissue() ? "true" : "false") << "\n"
       << "\t secondaryissue_threshold = " << std::to_string(get_secondaryissue_threshold()) << "\n"
       << "\t issuer = " << issuer << "\n"
       << "\t address = " << address << "\n"
       << "\t description = " << description << "\n";

    return ss.str();
}

bool token_detail::operator<(const token_detail &other) const
{
    typedef std::tuple<std::string, std::string> cmp_tuple;
    return cmp_tuple(symbol, issuer) < cmp_tuple(other.symbol, other.issuer);
}

const std::string &token_detail::get_symbol() const
{
    return symbol;
}
void token_detail::set_symbol(const std::string &symbol)
{
    size_t len = symbol.size() + 1 < (TOKEN_DETAIL_SYMBOL_FIX_SIZE) ? symbol.size() + 1 : TOKEN_DETAIL_SYMBOL_FIX_SIZE;
    this->symbol = symbol.substr(0, len);
}

uint64_t token_detail::get_maximum_supply() const
{
    return maximum_supply;
}
void token_detail::set_maximum_supply(uint64_t maximum_supply)
{
    this->maximum_supply = maximum_supply;
}

uint8_t token_detail::get_decimal_number() const
{
    return decimal_number;
}
void token_detail::set_decimal_number(uint8_t decimal_number)
{
    this->decimal_number = decimal_number;
}

const std::string &token_detail::get_issuer() const
{
    return issuer;
}
void token_detail::set_issuer(const std::string &issuer)
{
    size_t len = issuer.size() + 1 < (TOKEN_DETAIL_ISSUER_FIX_SIZE) ? issuer.size() + 1 : TOKEN_DETAIL_ISSUER_FIX_SIZE;
    this->issuer = issuer.substr(0, len);
}

const std::string &token_detail::get_address() const
{
    return address;
}
void token_detail::set_address(const std::string &address)
{
    size_t len = address.size() + 1 < (TOKEN_DETAIL_ADDRESS_FIX_SIZE) ? address.size() + 1 : TOKEN_DETAIL_ADDRESS_FIX_SIZE;
    this->address = address.substr(0, len);
}

const std::string &token_detail::get_description() const
{
    return description;
}
void token_detail::set_description(const std::string &description)
{
    size_t len = description.size() + 1 < (TOKEN_DETAIL_DESCRIPTION_FIX_SIZE) ? description.size() + 1 : TOKEN_DETAIL_DESCRIPTION_FIX_SIZE;
    this->description = description.substr(0, len);
}

std::vector<token_cert_type> token_detail::get_token_cert_mask() const
{
    std::vector<token_cert_type> certs;
    if (is_secondaryissue_legal())
    {
        certs.push_back(token_cert_ns::issue);
    }

    return certs;
}

bool token_detail::is_token_secondaryissue() const
{
    return secondaryissue_threshold >= 128;
}

void token_detail::set_token_secondaryissue()
{
    if (!is_token_secondaryissue())
    {
        secondaryissue_threshold += 128;
    }
}

uint8_t token_detail::get_secondaryissue_threshold() const
{
    if (is_token_secondaryissue())
        return secondaryissue_threshold - 128;
    else
        return secondaryissue_threshold;
}

void token_detail::set_secondaryissue_threshold(uint8_t share)
{
    BITCOIN_ASSERT(share < 128);
    secondaryissue_threshold = share;
}

bool token_detail::is_secondaryissue_threshold_value_ok() const
{
    return is_secondaryissue_threshold_value_ok(get_secondaryissue_threshold());
}

bool token_detail::is_secondaryissue_legal() const
{
    return is_secondaryissue_legal(get_secondaryissue_threshold());
}

bool token_detail::is_secondaryissue_forbidden(uint8_t threshold)
{
    return threshold == forbidden_secondaryissue_threshold;
}

bool token_detail::is_secondaryissue_freely(uint8_t threshold)
{
    return threshold == freely_secondaryissue_threshold;
}

bool token_detail::is_secondaryissue_threshold_value_ok(uint8_t threshold)
{
    return is_secondaryissue_forbidden(threshold) || is_secondaryissue_legal(threshold);
}

bool token_detail::is_secondaryissue_legal(uint8_t threshold)
{
    return is_secondaryissue_freely(threshold) || ((threshold >= 1) && (threshold <= 100));
}

bool token_detail::is_secondaryissue_owns_enough(uint64_t own, uint64_t total, uint8_t threshold)
{
    if (is_secondaryissue_freely(threshold))
        return true;
    if (!is_secondaryissue_legal(threshold))
        return false;
    uint64_t value_needed = (uint64_t)(((double)total) / 100 * threshold);
    return (value_needed == 0) || (own >= value_needed - 1); // allow 1 inaccurate
}

} // namespace chain
} // namespace libbitcoin
