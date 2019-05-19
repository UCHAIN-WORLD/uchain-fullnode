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
#ifndef UC_CHAIN_ASSET_WALLET_ADDRESS_HPP
#define UC_CHAIN_ASSET_WALLET_ADDRESS_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/coin/chain/point.hpp>
#include <UChain/coin/chain/script/script.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t ADDRESS_NAME_FIX_SIZE = 64;
BC_CONSTEXPR size_t ADDRESS_PRV_KEY_FIX_SIZE = 70;
BC_CONSTEXPR size_t ADDRESS_PUB_KEY_FIX_SIZE = 70;
BC_CONSTEXPR size_t ADDRESS_HD_INDEX_FIX_SIZE = 4;
BC_CONSTEXPR size_t ADDRESS_BALANCE_FIX_SIZE = 8;
BC_CONSTEXPR size_t ADDRESS_ALIAS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ADDRESS_ADDRESS_FIX_SIZE = 48;
BC_CONSTEXPR size_t ADDRESS_STATUS_FIX_SIZE = 1;

/// used for store wallet_address related information
enum wallet_address_status : uint8_t
{
    diabale = 0,
    enable = 1,        // common address
    multisig_addr = 2, // multisig address
    stealth_addr = 3   // stealth address
};

class BC_API wallet_address
{
  public:
    typedef std::vector<wallet_address> list;
    wallet_address();
    wallet_address(const std::string &name, const std::string &prv_key,
                   const std::string &pub_key, uint32_t hd_index, uint64_t balance,
                   const std::string &alias, const std::string &address, uint8_t status);
    wallet_address(const wallet_address &other);
    static wallet_address factory_from_data(const data_chunk &data);
    static wallet_address factory_from_data(std::istream &stream);
    static wallet_address factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
#ifdef UC_DEBUG
    std::string to_string();
#endif
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const std::string &get_name() const;
    void set_name(const std::string &name);
    const std::string get_prv_key(std::string &passphrase) const;
    const std::string get_prv_key() const;
    void set_prv_key(const std::string &prv_key, std::string &passphrase);
    void set_prv_key(const std::string &prv_key);
    const std::string &get_pub_key() const;
    void set_pub_key(const std::string &pub_key);
    uint32_t get_hd_index() const;
    void set_hd_index(uint32_t hd_index);
    uint64_t get_balance() const;
    void set_balance(uint64_t balance);
    const std::string &get_alias() const;
    void set_alias(const std::string &alias);
    const std::string &get_address() const;
    void set_address(const std::string &address);
    uint8_t get_status() const;
    void set_status(uint8_t status);

  private:
    std::string name;    // 64 bytes -- wallet name -- todo remove it later
    std::string prv_key; // 70 bytes
    std::string pub_key; // 70 bytes
    uint32_t hd_index;   // 4 bytes -- todo remove it later
    uint64_t balance;    // 8 bytes
    std::string alias;   // 64 bytes
    std::string address; // 48 bytes
    uint8_t status_;     // 1 bytes -- 0 -- diabale  1 -- enable
};

} // namespace chain
} // namespace libbitcoin

#endif
