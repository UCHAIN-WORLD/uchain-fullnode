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
#ifndef UC_CHAIN_ASSET_TOKEN_TRANSFER_HPP
#define UC_CHAIN_ASSET_TOKEN_TRANSFER_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <tuple>
#include <UChain/coin/chain/point.hpp>
#include <UChain/coin/chain/script/script.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/writer.hpp>
#include <UChain/coin/chain/history.hpp>

namespace libbitcoin
{
namespace chain
{

BC_CONSTEXPR size_t TOKEN_TRANSFER_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t TOKEN_TRANSFER_QUANTITY_FIX_SIZE = 8;

BC_CONSTEXPR size_t TOKEN_TRANSFER_FIX_SIZE = TOKEN_TRANSFER_SYMBOL_FIX_SIZE + TOKEN_TRANSFER_QUANTITY_FIX_SIZE;

struct token_balances
{
    typedef std::vector<token_balances> list;
    std::string symbol;
    std::string address;
    uint64_t unspent_token;
    uint64_t locked_token;

    // for sort
    bool operator<(const token_balances &other) const;
};

struct token_deposited_balance
{
    token_deposited_balance(const std::string &symbol_,
                            const std::string &address_,
                            const std::string &tx_hash_,
                            uint64_t tx_height_)
        : symbol(symbol_), address(address_), tx_hash(tx_hash_), tx_height(tx_height_), unspent_token(0), locked_token(0)
    {
    }

    std::string symbol;
    std::string address;
    std::string tx_hash;
    std::string model_param;
    uint64_t tx_height;
    uint64_t unspent_token;
    uint64_t locked_token;

    // for sort
    bool operator<(const token_deposited_balance &other) const
    {
        typedef std::tuple<std::string, uint64_t> cmp_tuple;
        return cmp_tuple(symbol, tx_height) < cmp_tuple(other.symbol, other.tx_height);
    }

    typedef std::vector<token_deposited_balance> list;
};

class BC_API token_transfer
{
  public:
    token_transfer();
    token_transfer(const std::string &symbol, uint64_t quantity);
    static token_transfer factory_from_data(const data_chunk &data);
    static token_transfer factory_from_data(std::istream &stream);
    static token_transfer factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;

    std::string to_string() const;

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const std::string &get_symbol() const;
    void set_symbol(const std::string &symbol);
    uint64_t get_quantity() const;
    void set_quantity(uint64_t quantity);

  private:
    std::string symbol; // symbol  -- in block
    uint64_t quantity;  // -- in block
};

} // namespace chain
} // namespace libbitcoin

#endif
