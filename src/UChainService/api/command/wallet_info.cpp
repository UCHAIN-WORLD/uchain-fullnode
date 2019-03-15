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
#include <UChainService/api/command/wallet_info.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

#include <UChain/bitcoin/math/crypto.hpp>
#include <UChain/bitcoin.hpp>
#include <UChain/bitcoin/config/base16.hpp>
#include <UChain/bitcoin/math/checksum.hpp>

using namespace libbitcoin::wallet;
using namespace libbitcoin::config;

namespace libbitcoin
{
namespace chain
{

wallet_info::wallet_info(blockchain::block_chain_impl &blockchain, std::string &passphrase) : blockchain_(blockchain),
                                                                                              passphrase_(passphrase)
{
}
wallet_info::wallet_info(blockchain::block_chain_impl &blockchain, std::string &passphrase,
                         libbitcoin::chain::wallet &meta, std::vector<wallet_address> &addr_vec, std::vector<token_detail> &token_vec) : blockchain_(blockchain), passphrase_(passphrase), meta_(meta), addr_vec_(addr_vec), token_vec_(token_vec)
{
}

bool wallet_info::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool wallet_info::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool wallet_info::from_data(reader &source)
{
    meta_.from_data(source);

    wallet_address addr;
    uint32_t addr_size = source.read_4_bytes_little_endian();
    while (addr_size--)
    {
        addr.reset();
        addr.from_data(source);
        addr_vec_.push_back(addr);
    }

    token_detail detail;
    uint32_t token_size = source.read_4_bytes_little_endian();
    while (token_size--)
    {
        detail.reset();
        detail.from_data(source);
        token_vec_.push_back(detail);
    }

    return true;
}

data_chunk wallet_info::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size()); // serialized_size is not used
    return data;
}

void wallet_info::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void wallet_info::to_data(writer &sink) const
{
    meta_.to_data(sink);
    // wallet_address vector
    sink.write_4_bytes_little_endian(addr_vec_.size());
    if (addr_vec_.size())
    {
        for (auto &each : addr_vec_)
        {
            each.to_data(sink);
        }
    }
    // wallet token vector
    sink.write_4_bytes_little_endian(token_vec_.size());
    if (token_vec_.size())
    {
        for (auto &each : token_vec_)
        {
            each.to_data(sink);
        }
    }
}
wallet wallet_info::get_wallet() const
{
    return meta_;
}
std::vector<wallet_address> &wallet_info::get_wallet_address()
{
    return addr_vec_;
}
std::vector<token_detail> &wallet_info::get_wallet_token()
{
    return token_vec_;
}
void wallet_info::store(std::string &name, std::string &passwd)
{
    // restore wallet
    auto acc = std::make_shared<libbitcoin::chain::wallet>(meta_);
    auto mnemonic = acc->get_mnemonic();
    acc->set_name(name);
    acc->set_mnemonic(mnemonic, passwd);
    acc->set_passwd(passwd);
    blockchain_.store_wallet(acc);

    // restore wallet addresses
    std::string prv_key;
    for (auto &each : addr_vec_)
    {
        prv_key = each.get_prv_key();
        each.set_prv_key(prv_key, passwd);
        each.set_name(name);
        auto addr = std::make_shared<wallet_address>(each);
        blockchain_.store_wallet_address(addr);
    }

    // restore wallet token
    for (auto &each : token_vec_)
    {
        each.set_issuer(name);
        auto acc = std::make_shared<token_detail>(each);
        blockchain_.store_wallet_token(acc, name);
    }
}
void wallet_info::encrypt()
{
    auto src_data = to_data();
    append_checksum(src_data);
    data_chunk pass_chunk(passphrase_.begin(), passphrase_.end());
    aes256_common_encrypt(src_data, pass_chunk, data_);
}
void wallet_info::decrypt(std::string &hexcode)
{
    data_chunk pass_chunk(passphrase_.begin(), passphrase_.end());
    data_chunk encrypt_data = base16(hexcode);
    aes256_common_decrypt(encrypt_data, pass_chunk, data_);
    if (!verify_checksum(data_))
        throw std::logic_error{"error while decrypting the wallet."};
}
std::istream &operator>>(std::istream &input, wallet_info &self_ref)
{
    std::string hexcode;
    input >> hexcode;
    self_ref.decrypt(hexcode);

    data_chunk body(self_ref.data_.begin(), self_ref.data_.end() - libbitcoin::checksum_size);
    self_ref.from_data(body);

    return input;
}

std::ostream &operator<<(std::ostream &output, const wallet_info &self_ref)
{
    wallet_info &tmp_self_ref = const_cast<wallet_info &>(self_ref);
    tmp_self_ref.encrypt();
    //output << base16(self_ref.to_data());
    output << base16(self_ref.data_);
    return output;
}

} // namespace chain
} // namespace libbitcoin
