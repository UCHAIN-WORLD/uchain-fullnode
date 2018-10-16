/**
 * Copyright (c) 2011-2018 UChain developers 
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
#ifndef UC_CHAIN_UOUT_HPP
#define UC_CHAIN_UOUT_HPP

#include <cstdint>
#include <istream>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <UChainService/txs/token/token.hpp>
#include <UChainService/txs/token/token_cert.hpp>
#include <UChainService/txs/token/token_card.hpp>
#include <UChainService/txs/token/attenuation_model.hpp>
#include <UChainService/txs/uid/uid.hpp>
#include <UChainService/txs/ucn/ucn.hpp>
#include <UChainService/txs/ucn/ucn_award.hpp>
#include <UChainService/txs/message/message.hpp>

using namespace libbitcoin::chain;
#define TYPE2UINT32(kd)  (static_cast<typename std::underlying_type<uout::uout_type>::type>(kd))

#define UCN_TYPE        TYPE2UINT32(uout::uout_type::uout_ucn)
#define UCN_AWARD_TYPE  TYPE2UINT32(uout::uout_type::uout_ucn_award)
#define TOKEN_TYPE      TYPE2UINT32(uout::uout_type::uout_token)
#define MESSAGE_TYPE    TYPE2UINT32(uout::uout_type::uout_message)
#define UID_TYPE        TYPE2UINT32(uout::uout_type::uout_uid)
#define TOKEN_CERT_TYPE TYPE2UINT32(uout::uout_type::uout_token_cert)
#define TOKEN_CARD_TYPE  TYPE2UINT32(uout::uout_type::uout_token_card)

#define UID_ATTACH_VERIFY_VERSION       TYPE2UINT32(207)


namespace libbitcoin {
namespace chain {

class BC_API uout
{
public:

    enum class uout_type : uint32_t
    {
        uout_ucn, // ucn
        uout_ucn_award,
        uout_token,
        uout_message,
        uout_uid,
        uout_token_cert,
        uout_token_card
    };

    typedef boost::variant<
        ucn,
        ucn_award,
        token,
        blockchain_message,
        uid,
        token_cert,
        token_card
        > uout_data_type;

    uout();

    uout(const std::string& from_uid, const std::string& to_uid);

    template<class Type>
    uout(uint32_t type, uint32_t version, const Type& attach_data)
        : type(type), version(version), attach(attach_data)
    {}

    static uout factory_from_data(const data_chunk& data);
    static uout factory_from_data(std::istream& stream);
    static uout factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    std::string to_string() const;
    bool is_valid() const;
    bool is_valid_type() const;
    void reset();
    uint64_t serialized_size() const;

    uint32_t get_version() const;
    void set_version(uint32_t version);
    uint32_t get_type() const;
    void set_type(uint32_t type);

    std::string get_to_uid() const;
    void set_to_uid(const std::string& uid);

    std::string get_from_uid() const;
    void set_from_uid(const std::string& uid);

    template<class Type>
    void set_attach(const Type& attach)
    {
         this->attach = attach;
    };
    uout_data_type& get_attach();
    const uout_data_type& get_attach() const;

private:
    uint32_t version;
    uint32_t type;
    std::string touid;
    std::string fromuid;
    uout_data_type attach;
};

} // namespace chain
} // namespace libbitcoin

#endif

