/**
 * Copyright (c) 2011-2018 libbitcoin developers 
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
#ifndef UC_CHAIN_OUTPUT_HPP
#define UC_CHAIN_OUTPUT_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/error.hpp>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <UChainService/txs/asset.hpp> // added for token issue/transfer
#include <UChainService/txs/uid/uid.hpp>

// forward declaration
namespace libbitcoin {
namespace blockchain {
    class block_chain_impl;
}
}

namespace libbitcoin {
namespace chain {

class BC_API output
{
public:
    typedef std::vector<output> list;

    static output factory_from_data(const data_chunk& data);
    static output factory_from_data(std::istream& stream);
    static output factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();
    static bool is_valid_symbol(const std::string& symbol, uint32_t tx_version);
    static bool is_valid_uid_symbol(const std::string& symbol,  bool check_sensitive = false);
    static bool is_valid_candidate_symbol(const std::string& symbol,  bool check_sensitive = false);
    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    code check_asset_address(bc::blockchain::block_chain_impl& chain) const;
    std::string get_script_address() const;
    void reset();
    uint64_t serialized_size() const;
    uint64_t get_token_amount() const;
    std::string get_token_symbol() const;
    std::string get_token_issuer() const;
    std::string get_token_address() const;
    std::string get_token_cert_symbol() const;
    std::string get_token_candidate_symbol() const;
    std::string get_token_cert_owner() const;
    std::string get_token_cert_address() const;
    token_cert_type get_token_cert_type() const;
    const data_chunk& get_attenuation_model_param() const;
    bool is_token() const;
    bool is_vote() const;
    bool is_token_transfer() const;
    bool is_token_issue() const;
    bool is_token_secondaryissue() const;
    bool is_token_candidate() const;
    bool is_token_candidate_register() const;
    bool is_token_candidate_transfer() const;
    bool is_token_cert() const;
    bool is_token_cert_issue() const;
    bool is_token_cert_transfer() const;
    bool is_token_cert_autoissue() const;
    bool is_ucn() const;
    bool is_ucn_award() const;
    bool is_message() const;
    bool is_uid() const;
    bool is_uid_register() const;
    bool is_uid_transfer() const;
    bool is_fromuid_filled() const;
    bool is_touid_filled() const;
    bool is_uid_full_filled() const;
    token_detail get_token_detail() const;
    token_transfer get_token_transfer() const;
    token_cert get_token_cert() const;
    token_candidate get_token_candidate() const;
    std::string get_uid_symbol() const;
    std::string get_uid_address() const;
    uid get_uid() const;

    uint64_t value;
    chain::script script;
    asset attach_data; // added for token issue/transfer
};

struct BC_API output_info
{
    typedef std::vector<output_info> list;

    output_point point;
    uint64_t value;
};

} // namespace chain
} // namespace libbitcoin

#endif
