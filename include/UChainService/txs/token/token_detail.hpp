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
#ifndef UC_CHAIN_ASSET_TOKEN_DETAIL_HPP
#define UC_CHAIN_ASSET_TOKEN_DETAIL_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include "token_cert.hpp"

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t TOKEN_DETAIL_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_DETAIL_MAX_SUPPLY_FIX_SIZE = 8;
BC_CONSTEXPR size_t TOKEN_DETAIL_TOKEN_TYPE_FIX_SIZE = 4;
BC_CONSTEXPR size_t TOKEN_DETAIL_ISSUER_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_DETAIL_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_DETAIL_DESCRIPTION_FIX_SIZE = 64;

BC_CONSTEXPR size_t TOKEN_DETAIL_FIX_SIZE = TOKEN_DETAIL_SYMBOL_FIX_SIZE
            + TOKEN_DETAIL_MAX_SUPPLY_FIX_SIZE
            + TOKEN_DETAIL_TOKEN_TYPE_FIX_SIZE
            + TOKEN_DETAIL_ISSUER_FIX_SIZE
            + TOKEN_DETAIL_ADDRESS_FIX_SIZE
            + TOKEN_DETAIL_DESCRIPTION_FIX_SIZE;

class BC_API token_detail
{
public:
    typedef std::vector<token_detail> list;

    static BC_CONSTEXPR uint8_t forbidden_secondaryissue_threshold = 0;
    static BC_CONSTEXPR uint8_t freely_secondaryissue_threshold = 127;

    token_detail();
    token_detail(
        const std::string& symbol, uint64_t maximum_supply,
        uint8_t decimal_number, uint8_t threshold, const std::string& issuer,
        const std::string& address, const std::string& description);

    static token_detail factory_from_data(const data_chunk& data);
    static token_detail factory_from_data(std::istream& stream);
    static token_detail factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();
    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

    bool operator< (const token_detail& other) const;
    std::string to_string() const;

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);
    uint64_t get_maximum_supply() const;
    void set_maximum_supply(uint64_t maximum_supply);
    uint8_t get_decimal_number() const;
    void set_decimal_number(uint8_t decimal_number);
    const std::string& get_issuer() const;
    void set_issuer(const std::string& issuer);
    const std::string& get_address() const;
    void set_address(const std::string& address);
    const std::string& get_description() const;
    void set_description(const std::string& description);
    std::vector<token_cert_type> get_token_cert_mask() const;

    bool is_token_secondaryissue() const;
    void set_token_secondaryissue();
    uint8_t get_secondaryissue_threshold() const;
    void set_secondaryissue_threshold(uint8_t share);

    bool is_secondaryissue_threshold_value_ok() const;
    bool is_secondaryissue_legal() const;

    static bool is_secondaryissue_forbidden(uint8_t threshold);
    static bool is_secondaryissue_freely(uint8_t threshold);
    static bool is_secondaryissue_threshold_value_ok(uint8_t threshold);
    static bool is_secondaryissue_legal(uint8_t threshold);
    static bool is_secondaryissue_owns_enough(uint64_t own, uint64_t total, uint8_t threshold);

private:
    // NOTICE: ref CAssetDetail in transaction.h
    // token_detail and CAssetDetail should have the same size and order.
    // uint32_t token_type in CAssetDetail is divided into four uint8_t parts here.
    std::string symbol;
    uint64_t maximum_supply;
    uint8_t decimal_number;
    uint8_t secondaryissue_threshold;
    uint8_t unused2;
    uint8_t unused3;
    std::string issuer;
    std::string address;
    std::string description;
};

} // namespace chain
} // namespace libbitcoin

#endif

