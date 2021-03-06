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
#ifndef UC_CHAIN_ATTACH_TOKEN_HPP
#define UC_CHAIN_ATTACH_TOKEN_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/coin/chain/point.hpp>
#include <UChain/coin/chain/script/script.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <UChainService/txs/token/token_detail.hpp>
#include <UChainService/txs/token/token_transfer.hpp>

using namespace libbitcoin::chain;

#define TOKEN_STATUS2UINT32(kd) (static_cast<typename std::underlying_type<token::token_status>::type>(kd))

#define TOKEN_DETAIL_TYPE TOKEN_STATUS2UINT32(token::token_status::token_locked)
#define TOKEN_TRANSFERABLE_TYPE TOKEN_STATUS2UINT32(token::token_status::token_transferable)

namespace libbitcoin
{
namespace chain
{

class BC_API token
{
  public:
    enum class token_status : uint32_t
    {
        token_none,
        token_locked,
        token_transferable,
    };
    typedef boost::variant<token_detail, token_transfer> token_data_type;

    token();
    token(uint32_t status, const token_detail &detail);
    token(uint32_t status, const token_transfer &detail);
    static token factory_from_data(const data_chunk &data);
    static token factory_from_data(std::istream &stream);
    static token factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    std::string to_string() const;
    bool is_valid_type() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    uint32_t get_status() const;
    void set_status(uint32_t status);
    void set_data(const token_detail &detail);
    void set_data(const token_transfer &detail);
    token_data_type &get_data();
    const token_data_type &get_data() const;

  private:
    uint32_t status;
    token_data_type data;
};

} // namespace chain
} // namespace libbitcoin

#endif
