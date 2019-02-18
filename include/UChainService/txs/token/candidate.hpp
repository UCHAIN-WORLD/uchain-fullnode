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
#ifndef UC_CHAIN_ATTACH_TOKEN_CANDIDATE_HPP
#define UC_CHAIN_ATTACH_TOKEN_CANDIDATE_HPP

#include <cstdint>
#include <istream>
#include <set>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/error.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

#define CANDIDATE_STATUS2UINT32(kd) (static_cast<typename std::underlying_type<candidate::candidate_status>::type>(kd))

#define CANDIDATE_STATUS_NONE CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_none)
#define CANDIDATE_STATUS_REGISTER CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_register)
#define CANDIDATE_STATUS_TRANSFER CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_transfer)
#define CANDIDATE_STATUS_MAX CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_max)
#define CANDIDATE_STATUS_HISTORY CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_history)
#define CANDIDATE_STATUS_CURRENT CANDIDATE_STATUS2UINT32(candidate::candidate_status::candidate_status_current)

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t TOKEN_CANDIDATE_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CANDIDATE_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_CANDIDATE_CONTENT_FIX_SIZE = 256;
BC_CONSTEXPR size_t TOKEN_CANDIDATE_STATUS_FIX_SIZE = 1;

BC_CONSTEXPR size_t TOKEN_CANDIDATE_FIX_SIZE = (TOKEN_CANDIDATE_SYMBOL_FIX_SIZE + TOKEN_CANDIDATE_ADDRESS_FIX_SIZE + TOKEN_CANDIDATE_CONTENT_FIX_SIZE + TOKEN_CANDIDATE_STATUS_FIX_SIZE);

BC_CONSTEXPR size_t TOKEN_CANDIDATE_TRANSFER_FIX_SIZE = (TOKEN_CANDIDATE_FIX_SIZE - TOKEN_CANDIDATE_CONTENT_FIX_SIZE);

// output_height; timestamp; to_uid; candidate;
BC_CONSTEXPR size_t TOKEN_CANDIDATE_INFO_FIX_SIZE = 4 + 4 + 64 + TOKEN_CANDIDATE_TRANSFER_FIX_SIZE;

class BC_API candidate
{
  public:
    typedef std::vector<candidate> list;

    enum class candidate_status : uint8_t
    {
        candidate_status_none = 0,
        candidate_status_register = 1,
        candidate_status_transfer = 2,
        candidate_status_history = 3,
        candidate_status_current = 4,
        candidate_status_max = 5

    };

    candidate();
    candidate(const std::string &symbol, const std::string &address,
              const std::string &content, uint8_t status = 0);

    void reset();
    bool is_valid() const;
    bool operator<(const candidate &other) const;

    static candidate factory_from_data(const data_chunk &data);
    static candidate factory_from_data(std::istream &stream);
    static candidate factory_from_data(reader &source);

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_short_data() const;
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;

    std::string to_string() const;
    uint64_t serialized_size() const;
    uint64_t calc_size() const;
    uint64_t get_max_serialized_size() const;

    const std::string &get_symbol() const;
    void set_symbol(const std::string &symbol);

    const std::string &get_address() const;
    void set_address(const std::string &address);

    void set_content(const std::string &content);
    const std::string &get_content() const;

    void set_status(uint8_t status);
    uint8_t get_status() const;
    std::string get_status_name() const;

    static std::string status_to_string(uint8_t status);

    bool is_register_status() const;
    bool is_transfer_status() const;
    bool is_invalid_status() const;

  private:
    // NOTICE: ref CTokenCandidate in transaction.h
    // candidate and CTokenCandidate should have the same size and order.
    uint8_t status_;      // token status
    std::string symbol_;  // token name/symbol
    std::string address_; // address that owned token cert
    std::string content_; // the content of the token, only serialization in register.
};

struct BC_API candidate_info
{
    typedef std::vector<candidate_info> list;
    uint32_t output_height;
    uint32_t timestamp;
    uint32_t vote;
    //uint8_t status;
    std::string to_uid;
    libbitcoin::chain::candidate candidate;

    uint64_t serialized_size() const;
    void reset();
    bool operator<(const candidate_info &other) const;

    static candidate_info factory_from_data(reader &source);
    bool from_data(reader &source);
    data_chunk to_data() const;
    data_chunk to_short_data() const;
    // const uint8_t& get_status();
    // void set_status(const uint8_t&  st);
};

} // namespace chain
} // namespace libbitcoin

#endif
