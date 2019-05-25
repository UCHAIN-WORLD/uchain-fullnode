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
#include <UChainService/txs/token/token_cert.hpp>
#include <sstream>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChain/blockchain/validate_transaction.hpp>

namespace libbitcoin
{
namespace chain
{

#define TOKEN_SYMBOL_DELIMITER "."

token_cert::token_cert()
{
    reset();
}

token_cert::token_cert(const std::string &symbol, const std::string &owner,
                       const std::string &address, token_cert_type cert_type)
    : symbol_(symbol), owner_(owner), address_(address), cert_type_(cert_type), status_(TOKEN_CERT_NORMAL_TYPE)
{
}

void token_cert::reset()
{
    symbol_ = "";
    owner_ = "";
    address_ = "";
    cert_type_ = token_cert_ns::none;
    status_ = TOKEN_CERT_NORMAL_TYPE;
}

bool token_cert::is_valid() const
{
    return !(symbol_.empty() || owner_.empty() || (cert_type_ == token_cert_ns::none) || (calc_size() > TOKEN_CERT_FIX_SIZE));
}

bool token_cert::operator<(const token_cert &other) const
{
    typedef std::tuple<std::string, token_cert_type> cmp_tuple;
    return cmp_tuple(symbol_, cert_type_) < cmp_tuple(other.symbol_, other.cert_type_);
}

std::string token_cert::get_domain(const std::string &symbol)
{
    std::string domain("");
    auto &&tokens = bc::split(symbol, TOKEN_SYMBOL_DELIMITER, true);
    if (tokens.size() > 0)
    {
        domain = tokens[0];
    }
    return domain;
}

bool token_cert::is_valid_domain(const std::string &domain)
{
    return !domain.empty();
}

std::string token_cert::get_key(const std::string &symbol, const token_cert_type &bit)
{
    return std::string(symbol + ":^#`@:" + std::to_string(bit));
}

std::string token_cert::token_cert::get_key() const
{
    return get_key(symbol_, cert_type_);
}

token_cert token_cert::factory_from_data(const data_chunk &data)
{
    token_cert instance;
    instance.from_data(data);
    return instance;
}

token_cert token_cert::factory_from_data(std::istream &stream)
{
    token_cert instance;
    instance.from_data(stream);
    return instance;
}

token_cert token_cert::factory_from_data(reader &source)
{
    token_cert instance;
    instance.from_data(source);
    return instance;
}

bool token_cert::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool token_cert::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool token_cert::from_data(reader &source)
{
    reset();
    symbol_ = source.read_string();
    owner_ = source.read_string();
    address_ = source.read_string();
    cert_type_ = source.read_4_bytes_little_endian();
    status_ = source.read_byte();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk token_cert::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void token_cert::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void token_cert::to_data(writer &sink) const
{
    sink.write_string(symbol_);
    sink.write_string(owner_);
    sink.write_string(address_);
    sink.write_4_bytes_little_endian(cert_type_);
    sink.write_byte(status_);
}

uint64_t token_cert::calc_size() const
{
    return (symbol_.size() + 1) + (owner_.size() + 1) + (address_.size() + 1) + TOKEN_CERT_TYPE_FIX_SIZE + TOKEN_CERT_STATUS_FIX_SIZE;
}

uint64_t token_cert::serialized_size() const
{
    return std::min<uint64_t>(calc_size(), TOKEN_CERT_FIX_SIZE);
}

std::string token_cert::to_string() const
{
    std::ostringstream ss;
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t owner = " << owner_ << "\n";
    ss << "\t address = " << address_ << "\n";
    ss << "\t cert = " << get_type_name() << "\n";
    ss << "\t status = " << std::to_string(status_) << "\n";
    return ss.str();
}

const std::string &token_cert::get_symbol() const
{
    return symbol_;
}

void token_cert::set_symbol(const std::string &symbol)
{
    size_t len = std::min((symbol.size() + 1), TOKEN_CERT_SYMBOL_FIX_SIZE);
    symbol_ = symbol.substr(0, len);
}

uint8_t token_cert::get_status() const
{
    return status_;
}

void token_cert::set_status(uint8_t status)
{
    status_ = status;
}

bool token_cert::is_newly_generated() const
{
    return (status_ == TOKEN_CERT_ISSUE_TYPE) || (status_ == TOKEN_CERT_AUTOISSUE_TYPE);
}

const std::string &token_cert::get_owner() const
{
    return owner_;
}

void token_cert::set_owner(const std::string &owner)
{
    size_t len = std::min((owner.size() + 1), TOKEN_CERT_OWNER_FIX_SIZE);
    owner_ = owner.substr(0, len);
}

const std::string &token_cert::get_address() const
{
    return address_;
}

void token_cert::set_address(const std::string &address)
{
    size_t len = std::min((address.size() + 1), TOKEN_CERT_ADDRESS_FIX_SIZE);
    address_ = address.substr(0, len);
}

token_cert_type token_cert::get_type() const
{
    return cert_type_;
}

void token_cert::set_type(token_cert_type cert_type)
{
    cert_type_ = cert_type;
}

token_cert_type token_cert::get_certs() const
{
    return cert_type_;
}

void token_cert::set_certs(token_cert_type cert_type)
{
    cert_type_ = cert_type;
}

std::string token_cert::get_type_name() const
{
    return get_type_name(cert_type_);
}

const std::map<token_cert_type, std::string> &token_cert::get_type_name_map()
{
    static std::map<token_cert_type, std::string> static_type_name_map = {
        {token_cert_ns::issue, "issue"},
        {token_cert_ns::domain, "domain"},
        {token_cert_ns::naming, "naming"},

        {token_cert_ns::marriage, "marriage"},
        {token_cert_ns::kyc, "KYC"},
    };
    return static_type_name_map;
}

std::string token_cert::get_type_name(token_cert_type cert_type)
{
    BITCOIN_ASSERT(cert_type != token_cert_ns::none);

    const auto &type_name_map = get_type_name_map();
    auto iter = type_name_map.find(cert_type);
    if (iter != type_name_map.end())
    {
        return iter->second;
    }

    std::stringstream sstream;
    sstream << "0x" << std::hex << cert_type;
    std::string result = sstream.str();
    return result;
}

bool token_cert::test_certs(const std::vector<token_cert_type> &cert_vec, token_cert_type cert_type)
{
    BITCOIN_ASSERT(cert_type != token_cert_ns::none);

    auto iter = std::find(cert_vec.begin(), cert_vec.end(), cert_type);
    return iter != cert_vec.end();
}

bool token_cert::test_certs(const std::vector<token_cert_type> &total, const std::vector<token_cert_type> &parts)
{
    if (total.size() < parts.size())
    {
        return false;
    }

    for (auto &cert_type : parts)
    {
        if (!test_certs(total, cert_type))
        {
            return false;
        }
    }

    return true;
}

} // namespace chain
} // namespace libbitcoin
