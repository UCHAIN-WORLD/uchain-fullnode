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
#ifndef UC_CHAIN_ATTACH_TOKEN_CERT_HPP
#define UC_CHAIN_ATTACH_TOKEN_CERT_HPP

#include <cstdint>
#include <UChain/coin/define.hpp>
#include <UChain/coin/error.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>

#define TOKEN_CERT_STATUS2UINT32(kd) (static_cast<typename std::underlying_type<token_cert::token_cert_status>::type>(kd))

#define TOKEN_CERT_NORMAL_TYPE TOKEN_CERT_STATUS2UINT32(token_cert::token_cert_status::token_cert_normal)
#define TOKEN_CERT_ISSUE_TYPE TOKEN_CERT_STATUS2UINT32(token_cert::token_cert_status::token_cert_issue)
#define TOKEN_CERT_TRANSFER_TYPE TOKEN_CERT_STATUS2UINT32(token_cert::token_cert_status::token_cert_transfer)
#define TOKEN_CERT_AUTOISSUE_TYPE TOKEN_CERT_STATUS2UINT32(token_cert::token_cert_status::token_cert_autoissue)

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t TOKEN_CERT_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CERT_OWNER_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CERT_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CERT_TYPE_FIX_SIZE = 4;
BC_CONSTEXPR size_t TOKEN_CERT_STATUS_FIX_SIZE = 1;

BC_CONSTEXPR size_t TOKEN_CERT_FIX_SIZE = (TOKEN_CERT_SYMBOL_FIX_SIZE + TOKEN_CERT_OWNER_FIX_SIZE + TOKEN_CERT_ADDRESS_FIX_SIZE + TOKEN_CERT_TYPE_FIX_SIZE + TOKEN_CERT_STATUS_FIX_SIZE);

using token_cert_type = uint32_t;
namespace token_cert_ns
{
constexpr token_cert_type none = 0;
constexpr token_cert_type issue = 1;
constexpr token_cert_type domain = 2;
constexpr token_cert_type naming = 3;

constexpr token_cert_type custom = 0x80000000;
constexpr token_cert_type marriage = custom + 0;
constexpr token_cert_type kyc = custom + 1;
} // namespace token_cert_ns

class BC_API token_cert
{
  public:
    typedef std::vector<token_cert> list;

    enum class token_cert_status : uint8_t
    {
        token_cert_normal = 0,
        token_cert_issue = 1,
        token_cert_transfer = 2,
        token_cert_autoissue = 3,
    };

    token_cert();
    token_cert(const std::string &symbol, const std::string &owner,
               const std::string &address, token_cert_type certs);

    void reset();
    bool is_valid() const;
    bool operator<(const token_cert &other) const;

    static token_cert factory_from_data(const data_chunk &data);
    static token_cert factory_from_data(std::istream &stream);
    static token_cert factory_from_data(reader &source);

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;

    std::string to_string() const;
    uint64_t serialized_size() const;
    uint64_t calc_size() const;

    const std::string &get_symbol() const;
    void set_symbol(const std::string &symbol);

    uint8_t get_status() const;
    void set_status(uint8_t status);
    bool is_newly_generated() const;

    void set_owner(const std::string &owner);
    const std::string &get_owner() const;

    void set_address(const std::string &owner);
    const std::string &get_address() const;

    token_cert_type get_certs() const;
    void set_certs(token_cert_type certs);

    token_cert_type get_type() const;
    void set_type(token_cert_type cert_type);
    std::string get_type_name() const;

    // auxiliary functions
    std::string get_key() const;

    static const std::map<token_cert_type, std::string> &get_type_name_map();
    static std::string get_type_name(token_cert_type cert_type);
    static bool test_certs(const std::vector<token_cert_type> &total, const std::vector<token_cert_type> &parts);
    static bool test_certs(const std::vector<token_cert_type> &certs, token_cert_type cert_type);

    static std::string get_domain(const std::string &symbol);
    static bool is_valid_domain(const std::string &domain);
    static std::string get_key(const std::string &symbol, const token_cert_type &bit);

  private:
    // NOTICE: ref CTokenCert in transaction.h
    // token_cert and CTokenCert should have the same size and order.
    std::string symbol_;        // token name/symbol
    std::string owner_;         // token cert owner, an digital identity
    std::string address_;       // address that owned token cert
    token_cert_type cert_type_; // token certs
    uint8_t status_;            // token status
};

} // namespace chain
} // namespace libbitcoin

#endif
