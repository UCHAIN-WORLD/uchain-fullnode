/**
 * Copyright (c) 2018-2020 UChain developers 
 *
 * This file is part of UChainService.
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
#ifndef UC_CHAIN_ASSET_WALLET_HPP
#define UC_CHAIN_ASSET_WALLET_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <UChain/bitcoin/formats/base_16.hpp>

namespace libbitcoin
{
namespace chain
{

enum wallet_status : uint8_t
{
    //system status
    locked = 0,
    imported,
    normal,
    //use status
    login,
    logout,
    error,
};

enum wallet_priority : uint8_t
{
    // 0 -- admin user  1 -- common user
    administrator = 0,
    common_user = 1,
};

enum wallet_type : uint8_t
{
    common = 0,
    multisignature,
};

/// used for store wallet related information
class BC_API wallet_multisig
{
  public:
    typedef std::vector<wallet_multisig> list;

    wallet_multisig();
    wallet_multisig(uint32_t hd_index, uint8_t m, uint8_t n,
                    std::vector<std::string> &&cosigner_pubkeys, std::string &pubkey);

    void set_hd_index(uint32_t index);
    uint32_t get_hd_index() const;
    inline void increase_hd_index() { hd_index_++; };

    void set_index(uint32_t index);
    uint32_t get_index() const;

    void set_m(uint8_t m);
    uint8_t get_m() const;

    void set_n(uint8_t n);
    uint8_t get_n() const;

    const std::vector<std::string> &get_cosigner_pubkeys() const;
    void set_cosigner_pubkeys(std::vector<std::string> &&cosigner_pubkeys);

    std::string get_pub_key() const;
    void set_pub_key(std::string &pubkey);

    std::string get_description() const;
    void set_description(std::string &description);

    std::string get_address() const;
    void set_address(std::string &address);

    std::string get_multisig_script() const;

    bool from_data(reader &source);
    void to_data(writer &sink) const;

    uint64_t serialized_size() const;

    bool operator==(const wallet_multisig &other) const;
    void reset();

#ifdef UC_DEBUG
    std::string to_string();
#endif

  private:
    uint32_t hd_index_;
    uint32_t index_;
    uint8_t m_;
    uint8_t n_;
    std::string pubkey_;
    std::vector<std::string> cosigner_pubkeys_;
    std::string description_;
    std::string address_;
};

class BC_API wallet
{
  public:
    wallet();
    wallet(const std::string &name, const std::string &mnemonic, const hash_digest &passwd,
           uint32_t hd_index, uint8_t priority, uint8_t status, uint8_t type);

    static wallet factory_from_data(const data_chunk &data);
    static wallet factory_from_data(std::istream &stream);
    static wallet factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;

    bool is_valid() const;
    void reset();

    uint64_t serialized_size() const;

    operator bool() const;

    bool operator==(const libbitcoin::chain::wallet &other) const;

#ifdef UC_DEBUG
    std::string to_string();
#endif

    const std::string &get_name() const;
    void set_name(const std::string &name);

    const std::string &get_mnemonic(std::string &passphrase, std::string &decry_output) const;
    const std::string &get_mnemonic() const;
    void set_mnemonic(const std::string &mnemonic, std::string &passphrase);
    void set_mnemonic(const std::string &mnemonic);

    const hash_digest &get_passwd() const;

    void set_passwd(const std::string &outside_passwd)
    {
        //bc::decode_hash(passwd, outside_passwd);
        data_chunk data(outside_passwd.begin(), outside_passwd.end());
        set_passwd(sha256_hash(data));
    }

    void set_passwd(const hash_digest &passwd_hash)
    {
        passwd = passwd_hash;
    }

    uint32_t get_hd_index() const;
    void set_hd_index(const uint32_t hd_index);

    void increase_hd_index() { hd_index++; };

    uint8_t get_status() const;
    void set_status(const uint8_t status);

    uint8_t get_priority() const;
    void set_priority(const uint8_t priority);

    void set_type(uint8_t type);
    uint8_t get_type() const;

    const wallet_multisig::list &get_multisig_vec() const;
    void set_multisig_vec(wallet_multisig::list &&multisig);

    bool is_multisig_exist(const wallet_multisig &multisig);
    void set_multisig(const wallet_multisig &multisig);
    void modify_multisig(const wallet_multisig &multisig);
    void remove_multisig(const wallet_multisig &multisig);
    std::shared_ptr<wallet_multisig::list> get_multisig(const std::string &addr);

  private:
    std::string name;
    std::string mnemonic;
    hash_digest passwd;

    uint32_t hd_index;
    //uint16_t status; // old define
    uint8_t type;
    uint8_t status;
    uint8_t priority;

    // multisig fields
    wallet_multisig::list multisig_vec;
    //wallet_multisig multisig;
};

} // namespace chain
} // namespace libbitcoin

#endif
