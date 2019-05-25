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
#include <UChainService/txs/wallet/wallet.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>

#ifdef UC_DEBUG
#include <json/minijson_writer.hpp>
#endif

#include <UChain/coin/math/crypto.hpp>
#include <UChain/coin.hpp>
using namespace libbitcoin::wallet;

namespace libbitcoin
{
namespace chain
{

wallet_multisig::wallet_multisig()
    : hd_index_(0), m_(0), n_(0)
{
}

wallet_multisig::wallet_multisig(
    uint32_t hd_index, uint8_t m, uint8_t n,
    std::vector<std::string> &&cosigner_pubkeys, std::string &pubkey)
    : hd_index_(hd_index), m_(m), n_(n), cosigner_pubkeys_(std::move(cosigner_pubkeys)), pubkey_(pubkey)
{
}

void wallet_multisig::set_hd_index(uint32_t hd_index)
{
    hd_index_ = hd_index;
}

uint32_t wallet_multisig::get_hd_index() const
{
    return hd_index_;
}

void wallet_multisig::set_index(uint32_t index)
{
    index_ = index;
}

uint32_t wallet_multisig::get_index() const
{
    return index_;
}

void wallet_multisig::set_m(uint8_t m)
{
    m_ = m;
}

uint8_t wallet_multisig::get_m() const
{
    return m_;
}

void wallet_multisig::set_n(uint8_t n)
{
    n_ = n;
}

uint8_t wallet_multisig::get_n() const
{
    return n_;
}

const std::vector<std::string> &wallet_multisig::get_cosigner_pubkeys() const
{
    return cosigner_pubkeys_;
}

void wallet_multisig::set_cosigner_pubkeys(std::vector<std::string> &&cosigner_pubkeys)
{
    cosigner_pubkeys_ = std::move(cosigner_pubkeys);
    std::sort(cosigner_pubkeys_.begin(), cosigner_pubkeys_.end());
}

std::string wallet_multisig::get_pub_key() const
{
    return pubkey_;
}

void wallet_multisig::set_pub_key(std::string &pubkey)
{
    pubkey_ = pubkey;
}

std::string wallet_multisig::get_description() const
{
    return description_;
}

void wallet_multisig::set_description(std::string &description)
{
    description_ = description;
}

std::string wallet_multisig::get_address() const
{
    return address_;
}

void wallet_multisig::set_address(std::string &address)
{
    address_ = address;
}

bool wallet_multisig::from_data(reader &source)
{
    hd_index_ = source.read_4_bytes_little_endian();
    index_ = source.read_4_bytes_little_endian();
    m_ = source.read_byte();
    n_ = source.read_byte();
    pubkey_ = source.read_string();
    // read consigner pubkeys
    uint8_t size = source.read_byte();
    while (size--)
        cosigner_pubkeys_.push_back(source.read_string());

    description_ = source.read_string();
    address_ = source.read_string();

    return true;
}

void wallet_multisig::to_data(writer &sink) const
{
    sink.write_4_bytes_little_endian(hd_index_);
    sink.write_4_bytes_little_endian(index_);
    sink.write_byte(m_);
    sink.write_byte(n_);
    sink.write_string(pubkey_);
    sink.write_byte(cosigner_pubkeys_.size());

    for (auto &each : cosigner_pubkeys_)
    {
        sink.write_string(each);
    }

    //sink.write_string(std::string("02b66fcb1064d827094685264aaa90d0126861688932eafbd1d1a4ba149de3308b"));
    sink.write_string(description_);
    sink.write_string(address_);
}

uint64_t wallet_multisig::serialized_size() const
{
    uint64_t size = 4 + 4 + 1 + 1 + (pubkey_.size() + 9) + 1; // hd_index,index,m,n,pubkey,pubkey number

    for (auto &each : cosigner_pubkeys_)
    {
        size += (each.size() + 9);
    }
    size += (description_.size() + 9);
    size += (address_.size() + 9);
    return size;
}

bool wallet_multisig::operator==(const wallet_multisig &other) const
{
    if (hd_index_ != other.hd_index_ || m_ != other.m_ || n_ != other.n_ || pubkey_ != other.pubkey_)
    {
        return false;
    }

    auto &other_pubkeys = other.cosigner_pubkeys_;
    if (cosigner_pubkeys_.size() != other_pubkeys.size())
    {
        return false;
    }

    for (const auto &pubkey : cosigner_pubkeys_)
    {
        auto iter = std::find(other_pubkeys.begin(), other_pubkeys.end(), pubkey);
        if (iter == other_pubkeys.end())
        {
            return false;
        }
    }

    return true;
}

void wallet_multisig::reset()
{
    hd_index_ = 0;
    index_ = 0;
    m_ = 0;
    n_ = 0;
    pubkey_ = "";
    cosigner_pubkeys_.clear();
    description_ = "";
    address_ = "";
}

#ifdef UC_DEBUG
std::string wallet_multisig::to_string()
{
    std::ostringstream ss;

    ss << "\t hd_index = " << hd_index_ << "\n"
       << "\t index = " << index_ << "\n"
       << "\t m = " << m_ << "\n"
       << "\t n = " << n_ << "\n"
       << "\t pubkey = " << pubkey_ << "\n"
       << "\t description = " << description_ << "\n";
    for (auto &each : cosigner_pubkeys_)
        ss << "\t cosigner-pubkey = " << each << std::endl;
    return ss.str();
}
#endif

std::string wallet_multisig::get_multisig_script() const
{
    if (m_ == 0 && n_ == 0)
    {
        // not initialized
        return "";
    }

    std::ostringstream ss;
    ss << std::to_string(m_);
    for (auto &each : cosigner_pubkeys_)
        ss << " [ " << each << " ] ";
    ss << std::to_string(n_) << " checkmultisig";
    return ss.str();
}

wallet::wallet()
{
    reset();
}

wallet::wallet(
    const std::string &name, const std::string &mnemonic, const hash_digest &passwd,
    uint32_t hd_index, uint8_t priority, uint8_t status, uint8_t type)
    : name(name), mnemonic(mnemonic), passwd(passwd), hd_index(hd_index), priority(priority), status(status), type(type)
{
}

wallet wallet::factory_from_data(const data_chunk &data)
{
    wallet instance;
    instance.from_data(data);
    return instance;
}

wallet wallet::factory_from_data(std::istream &stream)
{
    wallet instance;
    instance.from_data(stream);
    return instance;
}

wallet wallet::factory_from_data(reader &source)
{
    wallet instance;
    instance.from_data(source);
    return instance;
}

bool wallet::is_valid() const
{
    return true;
}

void wallet::reset()
{
    this->name = "";
    this->mnemonic = "";
    //this->passwd = "";
    this->hd_index = 0;
    this->priority = wallet_priority::common_user; // 0 -- admin user  1 -- common user
    this->type = wallet_type::common;
    this->status = wallet_status::normal;
}

bool wallet::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool wallet::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool wallet::from_data(reader &source)
{
    reset();
    name = source.read_string();
    //mnemonic = source.read_string();

    // read encrypted mnemonic
    auto size = source.read_variable_uint_little_endian();
    data_chunk string_bytes = source.read_data(size);
    std::string result(string_bytes.begin(), string_bytes.end());
    mnemonic = result;

    passwd = source.read_hash();
    hd_index = source.read_4_bytes_little_endian();
    priority = source.read_byte();
    //status = source.read_2_bytes_little_endian();
    type = source.read_byte();
    status = source.read_byte();
    if (type == wallet_type::multisignature)
    {
        //multisig.from_data(source);
        wallet_multisig multisig;
        uint32_t size = source.read_4_bytes_little_endian();
        while (size--)
        {
            multisig.reset();
            multisig.from_data(source);
            multisig_vec.push_back(multisig);
        }
    }
    return true;
}

data_chunk wallet::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size()); // serialized_size is not used
    return data;
}

void wallet::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void wallet::to_data(writer &sink) const
{
    sink.write_string(name);
    sink.write_string(mnemonic);
    sink.write_hash(passwd);
    sink.write_4_bytes_little_endian(hd_index);
    sink.write_byte(priority);
    //sink.write_2_bytes_little_endian(status);
    sink.write_byte(type);
    sink.write_byte(status);
    if (type == wallet_type::multisignature)
    {
        //multisig.to_data(sink);
        sink.write_4_bytes_little_endian(multisig_vec.size());
        if (multisig_vec.size())
        {
            for (auto &each : multisig_vec)
            {
                each.to_data(sink);
            }
        }
    }
}

uint64_t wallet::serialized_size() const
{
    uint64_t size = name.size() + mnemonic.size() + passwd.size() + 4 + 1 + 2 + 2 * 9; // 2 string len
    if (type == wallet_type::multisignature)
    {
        //size += multisig.serialized_size();
        size += 4; // vector size
        for (auto &each : multisig_vec)
            size += each.serialized_size();
    }
    return size;
}

wallet::operator bool() const
{
    return (name.empty() || mnemonic.empty());
}

bool wallet::operator==(const libbitcoin::chain::wallet &other) const
{
    return ((name == other.get_name()) && (passwd == other.get_passwd()) && (mnemonic == other.get_mnemonic()));
}

#ifdef UC_DEBUG
std::string wallet::to_string()
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
       << "\t mnemonic = " << mnemonic << "\n"
       << "\t password = " << passwd.data() << "\n"
       << "\t hd_index = " << hd_index << "\n"
       << "\t priority = " << priority << "\n"
       << "\t type = " << type << "\n"
       << "\t status = " << status << "\n";
    if (type == wallet_type::multisignature)
    {
        for (auto &each : multisig_vec)
            ss << "\t\t" << each.to_string();
    }
    return ss.str();
}
#endif

const std::string &wallet::get_name() const
{
    return name;
}

void wallet::set_name(const std::string &name)
{
    this->name = name;
}

const std::string &wallet::get_mnemonic() const
{
    return mnemonic; // for wallet == operator
}

const std::string &wallet::get_mnemonic(std::string &passphrase, std::string &decry_output) const
{
    decrypt_string(mnemonic, passphrase, decry_output);
    return decry_output;
}

void wallet::set_mnemonic(const std::string &mnemonic, std::string &passphrase)
{
    if (!mnemonic.size())
        throw std::logic_error{"mnemonic size is 0"};
    if (!passphrase.size())
        throw std::logic_error{"invalid password!"};
    std::string encry_output("");

    encrypt_string(mnemonic, passphrase, encry_output);
    this->mnemonic = encry_output;
}

void wallet::set_mnemonic(const std::string &mnemonic)
{
    this->mnemonic = mnemonic;
}

const hash_digest &wallet::get_passwd() const
{
    return passwd;
}

uint32_t wallet::get_hd_index() const
{
    return hd_index;
}

void wallet::set_hd_index(uint32_t hd_index)
{
    this->hd_index = hd_index;
}

uint8_t wallet::get_type() const
{
    return type;
}

void wallet::set_type(uint8_t type)
{
    this->type = type;
}

uint8_t wallet::get_status() const
{
    return status;
}

void wallet::set_status(uint8_t status)
{
    this->status = status;
}

uint8_t wallet::get_priority() const
{
    return priority;
}

void wallet::set_priority(uint8_t priority)
{
    this->priority = priority;
}

const wallet_multisig::list &wallet::get_multisig_vec() const
{
    return multisig_vec;
}

void wallet::set_multisig_vec(wallet_multisig::list &&multisig_vec)
{
    this->multisig_vec = std::move(multisig_vec);
}

bool wallet::is_multisig_exist(const wallet_multisig &multisig)
{
    auto iter = std::find(multisig_vec.begin(), multisig_vec.end(), multisig);
    return iter != multisig_vec.end();
}

void wallet::set_multisig(const wallet_multisig &multisig)
{
    if (!is_multisig_exist(multisig))
    {
        multisig_vec.push_back(multisig);
    }
}

void wallet::remove_multisig(const wallet_multisig &multisig)
{
    for (auto it = multisig_vec.begin(); it != multisig_vec.end();)
    {
        if (*it == multisig)
        {
            it = multisig_vec.erase(it);
            break;
        }
        else
        {
            ++it;
        }
    }
}

std::shared_ptr<wallet_multisig::list> wallet::get_multisig(const std::string &addr)
{
    auto acc_vec = std::make_shared<wallet_multisig::list>();
    for (auto &each : multisig_vec)
    {
        if (addr == each.get_address())
        {
            acc_vec->push_back(each);
        }
    }
    return acc_vec;
}

void wallet::modify_multisig(const wallet_multisig &multisig)
{
    for (auto &each : multisig_vec)
    {
        if (each == multisig)
        {
            each = multisig;
            break;
        }
    }
}

} // namespace chain
} // namespace libbitcoin
