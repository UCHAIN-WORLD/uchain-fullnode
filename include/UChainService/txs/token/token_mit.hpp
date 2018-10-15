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
#ifndef UC_CHAIN_ATTACH_TOKEN_MIT_HPP
#define UC_CHAIN_ATTACH_TOKEN_MIT_HPP

#include <cstdint>
#include <istream>
#include <set>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/error.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

#define MIT_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<token_mit::mit_status>::type>(kd))

#define MIT_STATUS_NONE        MIT_STATUS2UINT32(token_mit::mit_status::mit_status_none)
#define MIT_STATUS_REGISTER    MIT_STATUS2UINT32(token_mit::mit_status::mit_status_register)
#define MIT_STATUS_TRANSFER    MIT_STATUS2UINT32(token_mit::mit_status::mit_status_transfer)
#define MIT_STATUS_MAX         MIT_STATUS2UINT32(token_mit::mit_status::mit_status_max)

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t TOKEN_MIT_SYMBOL_FIX_SIZE  = 64;
BC_CONSTEXPR size_t TOKEN_MIT_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_MIT_CONTENT_FIX_SIZE = 256;
BC_CONSTEXPR size_t TOKEN_MIT_STATUS_FIX_SIZE  = 1;

BC_CONSTEXPR size_t TOKEN_MIT_FIX_SIZE = (TOKEN_MIT_SYMBOL_FIX_SIZE
        + TOKEN_MIT_ADDRESS_FIX_SIZE + TOKEN_MIT_CONTENT_FIX_SIZE
        + TOKEN_MIT_STATUS_FIX_SIZE);

BC_CONSTEXPR size_t TOKEN_MIT_TRANSFER_FIX_SIZE = (
            TOKEN_MIT_FIX_SIZE - TOKEN_MIT_CONTENT_FIX_SIZE);

// output_height; timestamp; to_did; mit;
BC_CONSTEXPR size_t TOKEN_MIT_INFO_FIX_SIZE = 4 + 4 + 64 + TOKEN_MIT_TRANSFER_FIX_SIZE;

class BC_API token_mit
{
public:
    typedef std::vector<token_mit> list;

    enum class mit_status : uint8_t
    {
        mit_status_none     = 0,
        mit_status_register = 1,
        mit_status_transfer = 2,
        mit_status_max      = 3,
    };

    token_mit();
    token_mit(const std::string& symbol, const std::string& address,
              const std::string& content);

    void reset();
    bool is_valid() const;
    bool operator< (const token_mit& other) const;

    static token_mit factory_from_data(const data_chunk& data);
    static token_mit factory_from_data(std::istream& stream);
    static token_mit factory_from_data(reader& source);

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_short_data() const;
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

    std::string to_string() const;
    uint64_t serialized_size() const;
    uint64_t calc_size() const;
    uint64_t get_max_serialized_size() const;

    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);

    const std::string& get_address() const;
    void set_address(const std::string& address);

    void set_content(const std::string& content);
    const std::string& get_content() const;

    void set_status(uint8_t status);
    uint8_t get_status() const;
    std::string get_status_name() const;

    static std::string status_to_string(uint8_t status);

    bool is_register_status() const;
    bool is_transfer_status() const;
    bool is_invalid_status() const;

private:
    // NOTICE: ref CAssetMit in transaction.h
    // token_mit and CAssetMit should have the same size and order.
    uint8_t status_;        // token status
    std::string symbol_;    // token name/symbol
    std::string address_;   // address that owned token cert
    std::string content_;   // the content of the token, only serialization in register.
};

struct BC_API token_mit_info
{
    typedef std::vector<token_mit_info> list;
    uint32_t output_height;
    uint32_t timestamp;
    std::string to_did;
    token_mit mit;

    uint64_t serialized_size() const;
    void reset();
    bool operator< (const token_mit_info& other) const;

    static token_mit_info factory_from_data(reader& source);
    data_chunk to_data() const;
    data_chunk to_short_data() const;
};

} // namespace chain
} // namespace libbitcoin

#endif

