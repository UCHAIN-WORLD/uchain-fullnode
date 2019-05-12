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
#include <UChainService/txs/wallet/wallet_address.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

#ifdef UC_DEBUG
#include <json/minijson_writer.hpp>
#endif

#include <UChain/bitcoin.hpp>
using namespace libbitcoin::wallet;

namespace libbitcoin
{
namespace chain
{

wallet_address::wallet_address()
{
    reset();
}

wallet_address::wallet_address(
    const std::string &name, const std::string &prv_key,
    const std::string &pub_key, uint32_t hd_index, uint64_t balance,
    const std::string &alias, const std::string &address, uint8_t status) : name(name), prv_key(prv_key), pub_key(pub_key), hd_index(hd_index), balance(balance),
                                                                            alias(alias), address(address), status_(status)
{
}

wallet_address::wallet_address(const wallet_address &other)
{
    name = other.name;
    prv_key = other.prv_key;
    pub_key = other.pub_key;
    hd_index = other.hd_index;
    balance = other.balance;
    alias = other.alias;
    address = other.address;
    status_ = other.status_;
}
wallet_address wallet_address::factory_from_data(const data_chunk &data)
{
    wallet_address instance;
    instance.from_data(data);
    return instance;
}

wallet_address wallet_address::factory_from_data(std::istream &stream)
{
    wallet_address instance;
    instance.from_data(stream);
    return instance;
}

wallet_address wallet_address::factory_from_data(reader &source)
{
    wallet_address instance;
    instance.from_data(source);
    return instance;
}

bool wallet_address::is_valid() const
{
    return true;
}

void wallet_address::reset()
{
    name = "";
    prv_key = "";
    pub_key = "";
    hd_index = 0;
    balance = 0;
    alias = "";
    address = "";
    status_ = 0;
}

bool wallet_address::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool wallet_address::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool wallet_address::from_data(reader &source)
{
    reset();
    name = source.read_fixed_string(ADDRESS_NAME_FIX_SIZE);
    //prv_key = source.read_fixed_string(ADDRESS_PRV_KEY_FIX_SIZE);

    // read encrypted private key
    auto size = source.read_variable_uint_little_endian();
    data_chunk string_bytes = source.read_data(size);
    std::string result(string_bytes.begin(), string_bytes.end());
    prv_key = result;
    //log::trace("from_data prv")<<prv_key;

    pub_key = source.read_fixed_string(ADDRESS_PUB_KEY_FIX_SIZE);
    hd_index = source.read_4_bytes_little_endian();
    balance = source.read_8_bytes_little_endian();
    alias = source.read_fixed_string(ADDRESS_ALIAS_FIX_SIZE);
    address = source.read_fixed_string(ADDRESS_ADDRESS_FIX_SIZE);
    status_ = source.read_byte();
    return true;
}

data_chunk wallet_address::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void wallet_address::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void wallet_address::to_data(writer &sink) const
{
    sink.write_fixed_string(name, ADDRESS_NAME_FIX_SIZE);
    //sink.write_fixed_string(prv_key, ADDRESS_PRV_KEY_FIX_SIZE);
    sink.write_string(prv_key);
    sink.write_fixed_string(pub_key, ADDRESS_PUB_KEY_FIX_SIZE);
    sink.write_4_bytes_little_endian(hd_index);
    sink.write_8_bytes_little_endian(balance);
    sink.write_fixed_string(alias, ADDRESS_ALIAS_FIX_SIZE);
    sink.write_fixed_string(address, ADDRESS_ADDRESS_FIX_SIZE);
    sink.write_byte(status_);
}

uint64_t wallet_address::serialized_size() const
{
    return name.size() + prv_key.size() + pub_key.size() + 4 + 8 + alias.size() + address.size() + 1 + 5; // 5 "string length" byte
}

#ifdef UC_DEBUG
std::string wallet_address::to_string()
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
       << "\t prv_key = " << prv_key << "\n"
       << "\t pub_key = " << pub_key << "\n"
       << "\t hd_index = " << hd_index << "\n"
       << "\t balance = " << balance << "\n"
       << "\t alias = " << alias << "\n"
       << "\t address = " << address << "\n"
       << "\t status = " << status_ << "\n";

    return ss.str();
}
#endif

const std::string &wallet_address::get_name() const
{
    return name;
}
void wallet_address::set_name(const std::string &name)
{
    this->name = name;
}

const std::string wallet_address::get_prv_key(std::string &passphrase) const
{
    std::string decry_output("");

    decrypt_string(prv_key, passphrase, decry_output);
    return decry_output;
}
const std::string wallet_address::get_prv_key() const
{
    return prv_key;
}
void wallet_address::set_prv_key(const std::string &prv_key, std::string &passphrase)
{
    std::string encry_output("");

    encrypt_string(prv_key, passphrase, encry_output);
    this->prv_key = encry_output;
}
void wallet_address::set_prv_key(const std::string &prv_key)
{
    this->prv_key = prv_key;
}

const std::string &wallet_address::get_pub_key() const
{
    return pub_key;
}
void wallet_address::set_pub_key(const std::string &pub_key)
{
    this->pub_key = pub_key;
}

uint32_t wallet_address::get_hd_index() const
{
    return hd_index;
}
void wallet_address::set_hd_index(uint32_t hd_index)
{
    this->hd_index = hd_index;
}

uint64_t wallet_address::get_balance() const
{
    return balance;
}
void wallet_address::set_balance(uint64_t balance)
{
    this->balance = balance;
}

const std::string &wallet_address::get_alias() const
{
    return alias;
}
void wallet_address::set_alias(const std::string &alias)
{
    this->alias = alias;
}

const std::string &wallet_address::get_address() const
{
    return address;
}
void wallet_address::set_address(const std::string &address)
{
    this->address = address;
}

uint8_t wallet_address::get_status() const
{
    return status_;
}
void wallet_address::set_status(uint8_t status)
{
    status_ = status;
}

} // namespace chain
} // namespace libbitcoin
