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
#include <UChain/coin/chain/output.hpp>
#include <cctype>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/coin/utility/container_sink.hpp>
#include <UChain/coin/utility/container_source.hpp>
#include <UChain/coin/utility/istream_reader.hpp>
#include <UChain/coin/utility/ostream_writer.hpp>
#include <UChain/coin/wallet/payment_address.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin
{
namespace chain
{

output output::factory_from_data(const data_chunk &data)
{
    output instance;
    instance.from_data(data);
    return instance;
}

output output::factory_from_data(std::istream &stream)
{
    output instance;
    instance.from_data(stream);
    return instance;
}

output output::factory_from_data(reader &source)
{
    output instance;
    instance.from_data(source);
    return instance;
}
bool output::is_valid_symbol(const std::string &symbol, uint32_t tx_version)
{
    if (symbol.empty())
        return false;
    // length check
    if (symbol.length() > TOKEN_DETAIL_SYMBOL_FIX_SIZE)
        return false;
    // char check
    for (const auto &i : symbol)
    {
        if (!(std::isalnum(i) || i == '.'))
            return false;
    }
    if (tx_version >= transaction_version::check_uid_feature)
    {
        // upper char check
        if (symbol != boost::to_upper_copy(symbol))
        {
            return false;
        }
        // sensitive check
        if (bc::wallet::symbol::is_sensitive(symbol))
        {
            return false;
        }
    }
    return true;
}

bool output::is_valid_uid_symbol(const std::string &symbol, bool check_sensitive)
{
    if (symbol.empty())
        return false;
    // length check
    if (symbol.length() > UID_DETAIL_SYMBOL_FIX_SIZE)
        return false;
    // char check
    for (const auto &i : symbol)
    {
        if (!(std::isalnum(i) || i == '.' || i == '@' || i == '_'))
            return false;
    }

    if (check_sensitive)
    {
        // sensitive check
        std::string symbolupper = symbol;
        boost::to_upper(symbolupper);
        if (bc::wallet::symbol::is_sensitive(symbolupper))
            return false;
    }

    return true;
}

bool output::is_valid_candidate_symbol(const std::string &symbol, bool check_sensitive)
{
    if (symbol.empty())
        return false;
    // length check
    if (symbol.length() > TOKEN_CANDIDATE_SYMBOL_FIX_SIZE)
        return false;
    // char check
    for (const auto &i : symbol)
    {
        if (!(std::isalnum(i) || i == '.' || i == '@' || i == '_'))
            return false;
    }

    if (check_sensitive)
    {
        // sensitive check
        auto upper = boost::to_upper_copy(symbol);
        if (bc::wallet::symbol::is_sensitive(upper))
            return false;
    }

    /*try {
        const auto authority = libbitcoin::config::authority(symbol);
        if (!authority.to_network_address().is_routable()) {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }*/

    return true;
}

bool output::is_valid() const
{
    return (value != 0) || script.is_valid() || attach_data.is_valid(); // added for token issue/transfer
}

std::string output::get_script_address() const
{
    auto payment_address = bc::wallet::payment_address::extract(script);
    return payment_address.encoded();
}

code output::check_asset_address(bc::blockchain::block_chain_impl &chain) const
{
    bool is_token = false;
    bool is_uid = false;
    std::string asset_address;
    if (is_token_issue() || is_token_secondaryissue() || is_candidate())
    {
        asset_address = get_token_address();
        is_token = true;
    }
    else if (is_token_cert())
    {
        asset_address = get_token_cert_address();
        is_token = true;
    }
    else if (is_uid_register() || is_uid_transfer())
    {
        asset_address = get_uid_address();
        is_uid = true;
    }
    if (is_token || is_uid)
    {
        auto script_address = get_script_address();
        if (asset_address != script_address)
        {
            log::debug("output::check_asset_address")
                << (is_token ? "token" : "uid")
                << " asset address " << asset_address
                << " is not equal to script address " << script_address;
            if (is_token)
            {
                return error::token_address_not_match;
            }
            if (is_uid)
            {
                return error::uid_address_not_match;
            }
        }
    }
    return error::success;
}

void output::reset()
{
    value = 0;
    script.reset();
    attach_data.reset(); // added for token issue/transfer
}

bool output::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool output::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool output::from_data(reader &source)
{
    reset();

    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
        result = script.from_data(source, true,
                                  script::parse_mode::raw_data_fallback);

    /* begin added for token issue/transfer */
    if (result)
        result = attach_data.from_data(source);
    /* end added for token issue/transfer */

    if (!result)
        reset();

    return result;
}

data_chunk output::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    log::debug("output::to_data") << "data.size=" << data.size();
    log::debug("output::to_data") << "serialized_size=" << serialized_size();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void output::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void output::to_data(writer &sink) const
{
    sink.write_8_bytes_little_endian(value);
    script.to_data(sink, true);
    /* begin added for token issue/transfer */
    attach_data.to_data(sink);
    /* end added for token issue/transfer */
}

uint64_t output::serialized_size() const
{
    return 8 + script.serialized_size(true) + attach_data.serialized_size(); // added for token issue/transfer
}

std::string output::to_string(uint32_t flags) const
{
    std::ostringstream ss;

    ss << "\tvalue = " << value << "\n"
       << "\t" << script.to_string(flags) << "\n"
       << "\t" << attach_data.to_string() << "\n"; // added for token issue/transfer

    return ss.str();
}

uint64_t output::get_token_amount() const // for validate_transaction.cpp to calculate token transfer amount
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return detail_info.get_maximum_supply();
        }
        if (token_info.get_status() == TOKEN_TRANSFERABLE_TYPE)
        {
            auto trans_info = boost::get<token_transfer>(token_info.get_data());
            return trans_info.get_quantity();
        }
    }
    return 0;
}

bool output::is_token_transfer() const
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        return (token_info.get_status() == TOKEN_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_vote() const
{
    return is_token_transfer() && get_token_symbol() == UC_VOTE_TOKEN_SYMBOL;
}

bool output::is_uid_transfer() const
{
    if (attach_data.get_type() == UID_TYPE)
    {
        auto uid_info = boost::get<uid>(attach_data.get_attach());
        return (uid_info.get_status() == UID_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_token_issue() const
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return !detail_info.is_token_secondaryissue();
        }
    }
    return false;
}

bool output::is_token_secondaryissue() const
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return detail_info.is_token_secondaryissue();
        }
    }
    return false;
}

bool output::is_candidate() const
{
    return (attach_data.get_type() == TOKEN_CANDIDATE_TYPE);
}

bool output::is_fromuid_filled() const
{
    return attach_data.get_version() == UID_ASSET_VERIFY_VERSION && !attach_data.get_from_uid().empty();
}

bool output::is_touid_filled() const
{
    return attach_data.get_version() == UID_ASSET_VERIFY_VERSION && !attach_data.get_to_uid().empty();
}

bool output::is_uid_full_filled() const
{
    return attach_data.get_version() == UID_ASSET_VERIFY_VERSION && !attach_data.get_to_uid().empty() && !attach_data.get_from_uid().empty();
}

std::string output::get_from_uid() const
{
    if (attach_data.get_version() == UID_ASSET_VERIFY_VERSION)
    {
        attach_data.get_from_uid();
    }
    return "";
}

std::string output::get_to_uid() const
{
    if (attach_data.get_version() == UID_ASSET_VERIFY_VERSION)
    {
        return attach_data.get_to_uid();
    }
    else
        return "";
}

std::string output::get_candidate_symbol() const
{
    if (is_candidate())
    {
        auto candidate_info = boost::get<candidate>(attach_data.get_attach());
        return candidate_info.get_symbol();
    }
    return std::string("");
}

bool output::is_candidate_register() const
{
    if (is_candidate())
    {
        auto token_info = boost::get<candidate>(attach_data.get_attach());
        if (token_info.is_register_status())
        {
            return true;
        }
    }
    return false;
}

bool output::is_candidate_transfer() const
{
    if (is_candidate())
    {
        auto token_info = boost::get<candidate>(attach_data.get_attach());
        if (token_info.is_transfer_status())
        {
            return true;
        }
    }
    return false;
}

bool output::is_token_cert() const
{
    return (attach_data.get_type() == TOKEN_CERT_TYPE);
}

bool output::is_token_cert_autoissue() const
{
    if (attach_data.get_type() == TOKEN_CERT_TYPE)
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        if (cert_info.get_status() == TOKEN_CERT_AUTOISSUE_TYPE)
        {
            return true;
        }
    }
    return false;
}

bool output::is_token_cert_issue() const
{
    if (attach_data.get_type() == TOKEN_CERT_TYPE)
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        if (cert_info.get_status() == TOKEN_CERT_ISSUE_TYPE)
        {
            return true;
        }
    }
    return false;
}

bool output::is_token_cert_transfer() const
{
    if (attach_data.get_type() == TOKEN_CERT_TYPE)
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        if (cert_info.get_status() == TOKEN_CERT_TRANSFER_TYPE)
        {
            return true;
        }
    }
    return false;
}

bool output::is_token() const
{
    return (attach_data.get_type() == UC_TOKEN_TYPE);
}

bool output::is_uid() const
{
    return (attach_data.get_type() == UID_TYPE);
}

bool output::is_ucn() const
{
    return (attach_data.get_type() == UCN_TYPE);
}

bool output::is_ucn_award() const
{
    return (attach_data.get_type() == UCN_AWARD_TYPE);
}

bool output::is_message() const
{
    return (attach_data.get_type() == MESSAGE_TYPE);
}

std::string output::get_token_symbol() const // for validate_transaction.cpp to calculate token transfer amount
{
    if (is_token())
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return detail_info.get_symbol();
        }
        if (token_info.get_status() == TOKEN_TRANSFERABLE_TYPE)
        {
            auto trans_info = boost::get<token_transfer>(token_info.get_data());
            return trans_info.get_symbol();
        }
    }
    else if (is_candidate())
    {
        auto token_info = boost::get<candidate>(attach_data.get_attach());
        return token_info.get_symbol();
    }
    else if (is_token_cert())
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_token_issuer() const // for validate_transaction.cpp to calculate token transfer amount
{
    if (is_token())
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return detail_info.get_issuer();
        }
    }
    else if (is_candidate())
    {
        BITCOIN_ASSERT(false);
    }
    return std::string("");
}

std::string output::get_token_address() const // for validate_transaction.cpp to verify token address
{
    if (is_token())
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            auto detail_info = boost::get<token_detail>(token_info.get_data());
            return detail_info.get_address();
        }
    }
    else if (is_candidate())
    {
        auto token_info = boost::get<candidate>(attach_data.get_attach());
        return token_info.get_address();
    }
    return std::string("");
}

candidate output::get_candidate() const
{
    if (is_candidate())
    {
        return boost::get<candidate>(attach_data.get_attach());
    }
    log::error("output::get_candidate") << "Token type is not an candidate.";
    return candidate();
}

token_cert output::get_token_cert() const
{
    if (is_token_cert())
    {
        return boost::get<token_cert>(attach_data.get_attach());
    }
    log::error("output::get_token_cert") << "Token type is not an token_cert.";
    return token_cert();
}

std::string output::get_token_cert_symbol() const
{
    if (is_token_cert())
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_token_cert_owner() const
{
    if (is_token_cert())
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        return cert_info.get_owner();
    }
    return std::string("");
}

std::string output::get_token_cert_address() const
{
    if (is_token_cert())
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        return cert_info.get_address();
    }

    return std::string("");
}

token_cert_type output::get_token_cert_type() const
{
    if (is_token_cert())
    {
        auto cert_info = boost::get<token_cert>(attach_data.get_attach());
        return cert_info.get_type();
    }
    return token_cert_ns::none;
}

bool output::is_uid_register() const
{
    if (attach_data.get_type() == UID_TYPE)
    {
        auto uid_info = boost::get<uid>(attach_data.get_attach());
        return (uid_info.get_status() == UID_DETAIL_TYPE);
    }
    return false;
}

std::string output::get_uid_symbol() const // for validate_transaction.cpp to calculate uid transfer amount
{
    if (attach_data.get_type() == UID_TYPE)
    {
        auto uid_info = boost::get<uid>(attach_data.get_attach());
        auto detail_info = boost::get<uid_detail>(uid_info.get_data());
        return detail_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_uid_address() const // for validate_transaction.cpp to calculate uid transfer amount
{
    if (attach_data.get_type() == UID_TYPE)
    {
        auto uid_info = boost::get<uid>(attach_data.get_attach());
        auto detail_info = boost::get<uid_detail>(uid_info.get_data());
        return detail_info.get_address();
    }
    return std::string("");
}

uid output::get_uid() const
{
    if (attach_data.get_type() == UID_TYPE)
    {
        return boost::get<uid>(attach_data.get_attach());
    }
    return uid();
}

token_transfer output::get_token_transfer() const
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_TRANSFERABLE_TYPE)
        {
            return boost::get<token_transfer>(token_info.get_data());
        }
    }
    log::error("output::get_token_transfer") << "Token type is not token_transfer_TYPE.";
    return token_transfer();
}

token_detail output::get_token_detail() const
{
    if (attach_data.get_type() == UC_TOKEN_TYPE)
    {
        auto token_info = boost::get<token>(attach_data.get_attach());
        if (token_info.get_status() == TOKEN_DETAIL_TYPE)
        {
            return boost::get<token_detail>(token_info.get_data());
        }
    }
    log::error("output::get_token_detail") << "Token type is not TOKEN_DETAIL_TYPE.";
    return token_detail();
}

const data_chunk &output::get_attenuation_model_param() const
{
    BITCOIN_ASSERT(operation::is_pay_key_hash_with_attenuation_model_pattern(script.operations));
    return operation::get_model_param_from_pay_key_hash_with_attenuation_model(script.operations);
}

} // namespace chain
} // namespace libbitcoin
