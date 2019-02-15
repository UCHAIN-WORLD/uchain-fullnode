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
#pragma once

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <UChain/bitcoin/formats/base_16.hpp>
#include <UChain/blockchain/block_chain_impl.hpp>
#include <UChainService/txs/wallet/wallet_address.hpp>
#include <UChainService/txs/token/token_detail.hpp>

namespace libbitcoin
{
namespace chain
{

// used for store all wallet related information
class BC_API wallet_info
{
  public:
    wallet_info(libbitcoin::blockchain::block_chain_impl &blockchain, std::string &passphrase);
    wallet_info(libbitcoin::blockchain::block_chain_impl &blockchain, std::string &passphrase,
                libbitcoin::chain::wallet &meta, std::vector<wallet_address> &addr_vec, std::vector<token_detail> &token_vec);

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    void store(std::string &name, std::string &passwd);
    wallet get_wallet() const;
    std::vector<wallet_address> &get_wallet_address();
    std::vector<token_detail> &get_wallet_token();
    void encrypt();
    void decrypt(std::string &hexcode);
    friend std::istream &operator>>(std::istream &input, wallet_info &self_ref);
    friend std::ostream &operator<<(std::ostream &output, const wallet_info &self_ref);

  private:
    libbitcoin::blockchain::block_chain_impl &blockchain_;
    wallet meta_;
    std::vector<wallet_address> addr_vec_;
    std::vector<token_detail> token_vec_;
    // encrypt/decrypt
    data_chunk data_;
    std::string passphrase_;
};

} // namespace chain
} // namespace libbitcoin
