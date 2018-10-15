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
#ifndef UC_CHAIN_ATTACH_TOKEN_CARD_HPP
#define UC_CHAIN_ATTACH_TOKEN_CARD_HPP

#include <cstdint>
#include <istream>
#include <set>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/error.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

#define CARD_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<token_card::card_status>::type>(kd))

#define CARD_STATUS_NONE        CARD_STATUS2UINT32(token_card::card_status::card_status_none)
#define CARD_STATUS_REGISTER    CARD_STATUS2UINT32(token_card::card_status::card_status_register)
#define CARD_STATUS_TRANSFER    CARD_STATUS2UINT32(token_card::card_status::card_status_transfer)
#define CARD_STATUS_MAX         CARD_STATUS2UINT32(token_card::card_status::card_status_max)

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t TOKEN_CARD_SYMBOL_FIX_SIZE  = 64;
BC_CONSTEXPR size_t TOKEN_CARD_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CARD_CONTENT_FIX_SIZE = 256;
BC_CONSTEXPR size_t TOKEN_CARD_STATUS_FIX_SIZE  = 1;

BC_CONSTEXPR size_t TOKEN_CARD_FIX_SIZE = (TOKEN_CARD_SYMBOL_FIX_SIZE
        + TOKEN_CARD_ADDRESS_FIX_SIZE + TOKEN_CARD_CONTENT_FIX_SIZE
        + TOKEN_CARD_STATUS_FIX_SIZE);

BC_CONSTEXPR size_t TOKEN_CARD_TRANSFER_FIX_SIZE = (
            TOKEN_CARD_FIX_SIZE - TOKEN_CARD_CONTENT_FIX_SIZE);

// output_height; timestamp; to_uid; mit;
BC_CONSTEXPR size_t TOKEN_CARD_INFO_FIX_SIZE = 4 + 4 + 64 + TOKEN_CARD_TRANSFER_FIX_SIZE;

class BC_API token_card
{
public:
    typedef std::vector<token_card> list;

    enum class card_status : uint8_t
    {
        card_status_none     = 0,
        card_status_register = 1,
        card_status_transfer = 2,
        card_status_max      = 3,
    };

    token_card();
    token_card(const std::string& symbol, const std::string& address,
              const std::string& content);

    void reset();
    bool is_valid() const;
    bool operator< (const token_card& other) const;

    static token_card factory_from_data(const data_chunk& data);
    static token_card factory_from_data(std::istream& stream);
    static token_card factory_from_data(reader& source);

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
    // token_card and CAssetMit should have the same size and order.
    uint8_t status_;        // token status
    std::string symbol_;    // token name/symbol
    std::string address_;   // address that owned token cert
    std::string content_;   // the content of the token, only serialization in register.
};

struct BC_API token_card_info
{
    typedef std::vector<token_card_info> list;
    uint32_t output_height;
    uint32_t timestamp;
    std::string to_uid;
    token_card mit;

    uint64_t serialized_size() const;
    void reset();
    bool operator< (const token_card_info& other) const;

    static token_card_info factory_from_data(reader& source);
    data_chunk to_data() const;
    data_chunk to_short_data() const;
};

} // namespace chain
} // namespace libbitcoin

#endif

