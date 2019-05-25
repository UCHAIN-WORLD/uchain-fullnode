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
#ifndef UC_CHAIN_TRANSACTION_HPP
#define UC_CHAIN_TRANSACTION_HPP

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include <UChain/coin/define.hpp>
#include <UChain/coin/chain/input.hpp>
#include <UChain/coin/chain/output.hpp>
#include <UChain/coin/math/elliptic_curve.hpp>
#include <UChain/coin/utility/reader.hpp>
#include <UChain/coin/utility/thread.hpp>
#include <UChain/coin/utility/writer.hpp>

namespace libbitcoin
{
namespace chain
{

enum transaction_version
{
    first = 1,               //the frist version
    check_output_script = 2, //add check output script
    check_uid_testnet = 3,
    check_uid_feature = 4,
    max_version = 5
};

class BC_API transaction
{
  public:
    typedef std::vector<transaction> list;
    typedef std::shared_ptr<transaction> ptr;
    typedef std::vector<ptr> ptr_list;
    typedef std::vector<size_t> indexes;

    static transaction factory_from_data(const data_chunk &data);
    static transaction factory_from_data(std::istream &stream);
    static transaction factory_from_data(reader &source);
    static uint64_t satoshi_fixed_size();

    transaction();
    transaction(const transaction &other);
    transaction(uint32_t version, uint32_t locktime, const input::list &inputs,
                const output::list &outputs);

    transaction(transaction &&other);
    transaction(uint32_t version, uint32_t locktime, input::list &&inputs,
                output::list &&outputs);

    /// This class is move assignable [but not copy assignable].
    transaction &operator=(transaction &&other);

    // TODO: eliminate blockchain transaction copies and then delete this.
    transaction &operator=(const transaction &other) /*= delete*/;

    bool from_data(const data_chunk &data);
    bool from_data(std::istream &stream);
    bool from_data(reader &source);
    data_chunk to_data() const;
    void to_data(std::ostream &stream) const;
    void to_data(writer &sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    void reset();
    hash_digest hash() const;

    // sighash_type is used by OP_CHECKSIG
    hash_digest hash(uint32_t sighash_type) const;
    bool is_coinbase() const;
    bool is_strict_coinbase() const;
    bool is_token_block_coinbase() const;
    bool is_final(uint64_t block_height, uint32_t block_time) const;
    bool is_locktime_conflict() const;
    uint64_t total_output_value() const;
    uint64_t serialized_size() const;
    uint64_t total_output_transfer_amount() const;
    uint64_t total_output_vote_amount() const;
    bool has_token_issue() const;
    bool has_token_secondary_issue() const;
    bool has_token_transfer() const;
    bool has_token_vote() const;
    bool has_token_cert() const;
    bool has_candidate_transfer() const;
    bool has_candidate_register() const;

    bool has_uid_register() const;
    bool has_uid_transfer() const;
    std::string get_uid_transfer_old_address() const;

    uint32_t version;
    uint32_t locktime;
    input::list inputs;
    output::list outputs;

  private:
    mutable upgrade_mutex mutex_;
    mutable std::shared_ptr<hash_digest> hash_;
};

} // namespace chain
} // namespace libbitcoin

#endif
