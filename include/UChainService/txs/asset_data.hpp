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
#ifndef UC_CHAIN_BUSINESS_DATA_HPP
#define UC_CHAIN_BUSINESS_DATA_HPP

#include <cstdint>
#include <istream>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <UChainService/txs/token/token_transfer.hpp>
#include <UChainService/txs/token/token_cert.hpp>
#include <UChainService/txs/token/token_candidate.hpp>
#include <UChainService/txs/uid/uid_detail.hpp>
#include <UChainService/txs/ucn/ucn.hpp>
#include <UChainService/txs/ucn/ucn_award.hpp>
#include <UChainService/txs/message/message.hpp>

#define KIND2UINT16(kd)  (static_cast<typename std::underlying_type<business_kind>::type>(kd))
// 0 -- unspent  1 -- confirmed  2 -- local token not issued
#define TOKEN_STATUS_UNSPENT    0 // in blockchain
#define TOKEN_STATUS_CONFIRMED  1 // in blockchain
#define TOKEN_STATUS_UNISSUED   2 // in local database

namespace libbitcoin {
namespace chain {

enum class business_kind : uint16_t
{
    ucn = 0,
    token_issue = 1,
    token_transfer = 2,
    message = 3,
    ucn_award = 4, // store to address_token database
    uid_register = 5,
    uid_transfer = 6,
    token_cert = 7,
    token_candidate = 8,
    unknown = 0xffff
};

// 0 -- unspent  1 -- confirmed  2 -- local token not issued
enum business_status : uint8_t
{
    unspent = 0, // in blockchain but unspent
    confirmed = 1, // in blockchain confirmed
    unissued = 2, //  in local database ,special for token related business
    unknown = 0xff
};

class BC_API asset_data
{
public:
    typedef boost::variant<
        ucn,
        ucn_award,
        token_detail,
        token_transfer,
        token_cert,
        token_candidate,
        blockchain_message,
        uid_detail> asset_data_type;

    static asset_data factory_from_data(const data_chunk& data);
    static asset_data factory_from_data(std::istream& stream);
    static asset_data factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() ;
    void to_data(std::ostream& stream) ;
    void to_data(writer& sink);
#if UC_DEBUG
    std::string to_string() ;
#endif
    bool is_valid() const;
    bool is_valid_type() const;
    void reset();
    uint64_t serialized_size() ;
    business_kind get_kind_value() const;
    const asset_data_type& get_data() const;
    uint32_t get_timestamp() const;

private:
    business_kind kind; // 2 size
    uint32_t timestamp; // 4 size
    asset_data_type data;

};

/*************************************business assisant class begin******************************************/
class BC_API business_record
{
public:
    typedef std::vector<business_record> list;

    // The type of point (output or spend).
    point_kind kind;

    /// The point that identifies the record.
    chain::point point;

    /// The height of the point.
    uint64_t height;

    union
    {
        /// If output, then satoshi value of output.
        uint64_t value;

        /// If spend, then checksum hash of previous output point
        /// To match up this row with the output, recompute the
        /// checksum from the output row with spend_checksum(row.point)
        uint64_t previous_checksum;
    } val_chk_sum;

    asset_data data;

#ifdef UC_DEBUG
    // just used for debug code in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t kind = " << KIND2UINT16(kind)
            << "\t point = " << point.to_string() << "\n"
            << "\t height = " << height
            << "\t data = " << data.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_history
{
public:
    typedef std::vector<business_history> list;
    /// If there is no output this is null_hash:max.
    output_point output;
    uint64_t output_height;

    /// The satoshi value of the output.
    uint64_t value;

    /// If there is no spend this is null_hash:max.
    input_point spend;

    union
    {
        /// The height of the spend or max if no spend.
        uint64_t spend_height;

        /// During expansion this value temporarily doubles as a checksum.
        uint64_t temporary_checksum;
    };
    uint32_t status; // 0 -- unspend  1 -- confirmed
    asset_data data;  // for output only

#ifdef UC_DEBUG
    // just used for debug code in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t output = " << output.to_string()
            << "\t output_height = " << output_height
            << "\t value = " << value << "\n"
            << "\t spend = " << spend.to_string()
            << "\t data = " << data.to_string() << "\n";

        return ss.str();
    }
#endif
};
class BC_API business_address_token
{
public:
    typedef std::vector<business_address_token> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local token not issued
    uint64_t quantity;
    token_detail detail;

#ifdef UC_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t quantity = " << quantity << "\n"
            << "\t detail = " << detail.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_token_cert
{
public:
    typedef std::vector<business_address_token_cert> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local token not issued
    token_cert certs;

#ifdef UC_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t certs = " << certs.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_candidate
{
public:
    typedef std::vector<business_address_candidate> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local token not issued
    token_candidate candidate;

#ifdef UC_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << std::to_string(status)
            << "\t candidate = " << candidate.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_uid
{
public:
    typedef std::vector<business_address_uid> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local token not issued
    uid_detail detail;

#ifdef UC_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t detail = " << detail.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_message
{
public:
    typedef std::vector<business_address_message> list;

    std::string  address;
    uint8_t status;
    chain::blockchain_message msg;
#ifdef UC_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t message = " << msg.to_string() << "\n";

        return ss.str();
    }
#endif
};

/****************************************business assisant class end*******************************************/

} // namespace chain
} // namespace libbitcoin

#endif

