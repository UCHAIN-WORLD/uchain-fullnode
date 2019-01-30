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
#ifndef UC_CHAIN_OPERATION_HPP
#define UC_CHAIN_OPERATION_HPP

#include <cstddef>
#include <iostream>
#include <vector>
#include <UChain/bitcoin/chain/script/opcode.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/math/elliptic_curve.hpp>
#include <UChain/bitcoin/utility/data.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>

namespace libbitcoin
{
namespace chain
{

// forward declaration
class point;

/// Script patterms.
/// Comments from: bitcoin.org/en/developer-guide#signature-hash-types
enum class script_pattern
{
    /// Null Data
    /// Pubkey Script: OP_RETURN <0 to 80 bytes of data> (formerly 40 bytes)
    /// Null data scripts cannot be spent, so there's no signature script.
    null_data,

    /// Pay to Multisig [BIP11]
    /// Pubkey script: <m> <A pubkey>[B pubkey][C pubkey...] <n> OP_CHECKMULTISIG
    /// Signature script: OP_0 <A sig>[B sig][C sig...]
    pay_multisig,

    /// Pay to Public Key (obsolete)
    pay_public_key,

    /// Pay to Public Key Hash [P2PKH]
    /// Pubkey script: OP_DUP OP_HASH160 <PubKeyHash> OP_EQUALVERIFY OP_CHECKSIG
    /// Signature script: <sig> <pubkey>
    pay_key_hash,

    pay_key_hash_with_lock_height,

    /// Pay to Script Hash [P2SH/BIP16]
    /// The redeem script may be any pay type, but only multisig makes sense.
    /// Pubkey script: OP_HASH160 <Hash160(redeemScript)> OP_EQUAL
    /// Signature script: <sig>[sig][sig...] <redeemScript>
    pay_script_hash,

    /// Sign Multisig script [BIP11]
    sign_multisig,

    /// Sign Public Key (obsolete)
    sign_public_key,

    /// Sign Public Key Hash [P2PKH]
    sign_key_hash,

    sign_key_hash_with_lock_height,

    /// Sign Script Hash [P2SH/BIP16]
    sign_script_hash,

    /// The script is valid but does not conform to the standard tempaltes.
    /// Such scripts are always accepted if they are mined into blocks, but
    /// transactions with non-standard scripts may not be forwarded by peers.
    non_standard,

    /// Pay to black hole address
    pay_blackhole_address,

    /// Pay to Public Key Hash [P2PKH] with attenuation model
    /// Pubkey script:
    ///     <model_param> <input_point> OP_CHECKATTENUATIONVERIFY
    ///     OP_DUP OP_HASH160 <PubKeyHash> OP_EQUALVERIFY OP_CHECKSIG
    /// Signature script: <sig> <pubkey>
    pay_key_hash_with_attenuation_model,

};

class BC_API operation
{
  public:
    typedef std::vector<operation> stack;

    static const size_t max_null_data_size;

    static operation factory_from_data(const data_chunk &data);
    static operation factory_from_data(std::istream &stream);
    static operation factory_from_data(reader &source);

    static bool is_push_only(const operation::stack &operations);

    /// unspendable pattern (standard)
    static bool is_null_data_pattern(const operation::stack &ops);

    /// payment script patterns (standard)
    static bool is_pay_multisig_pattern(const operation::stack &ops);
    static bool is_pay_public_key_pattern(const operation::stack &ops);
    static bool is_pay_key_hash_pattern(const operation::stack &ops);
    static bool is_pay_key_hash_with_lock_height_pattern(const operation::stack &ops);
    static bool is_pay_script_hash_pattern(const operation::stack &ops);
    static bool is_pay_blackhole_pattern(const operation::stack &ops);
    static bool is_pay_key_hash_with_attenuation_model_pattern(const operation::stack &ops);

    /// signature script patterns (standard)
    static bool is_sign_multisig_pattern(const operation::stack &ops);
    static bool is_sign_public_key_pattern(const operation::stack &ops);
    static bool is_sign_key_hash_pattern(const operation::stack &ops);
    static bool is_sign_key_hash_with_lock_height_pattern(const operation::stack &ops);
    static bool is_sign_script_hash_pattern(const operation::stack &ops);

    static uint64_t get_lock_height_from_sign_key_hash_with_lock_height(const operation::stack &ops);
    static uint64_t get_lock_height_from_pay_key_hash_with_lock_height(const operation::stack &ops);
    static const data_chunk &get_model_param_from_pay_key_hash_with_attenuation_model(const operation::stack &ops);
    static const data_chunk &get_input_point_from_pay_key_hash_with_attenuation_model(const operation::stack &ops);

    /// stack factories
    static stack to_null_data_pattern(data_slice data);
    static stack to_pay_multisig_pattern(uint8_t signatures,
                                         const point_list &points);
    static stack to_pay_multisig_pattern(uint8_t signatures,
                                         const data_stack &points);
    static stack to_pay_public_key_pattern(data_slice point);
    static stack to_pay_key_hash_pattern(const short_hash &hash);
    static stack to_pay_key_hash_with_lock_height_pattern(const short_hash &hash, uint32_t block_height);
    static stack to_pay_script_hash_pattern(const short_hash &hash);
    static stack to_pay_blackhole_pattern(const short_hash &hash);
    static stack to_pay_key_hash_with_attenuation_model_pattern(
        const short_hash &hash, const std::string &model_param, const point &input_point);

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;

    opcode code;
    data_chunk data;

  private:
    static bool is_push(const opcode code);
    static uint64_t count_non_push(const operation::stack &operations);
    static bool must_read_data(opcode code);
    static bool read_opcode_data_size(uint32_t &count, opcode code,
                                      uint8_t byte, reader &source);
};

} // namespace chain
} // namespace libbitcoin

#endif
