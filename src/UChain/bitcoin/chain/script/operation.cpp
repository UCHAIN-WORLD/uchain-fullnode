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
#include <script/script.h>
#include <UChain/bitcoin/chain/script/operation.hpp>
#include <algorithm>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/formats/base_16.hpp>
#include <UChain/bitcoin/math/elliptic_curve.hpp>
#include <UChain/bitcoin/utility/container_sink.hpp>
#include <UChain/bitcoin/utility/container_source.hpp>
#include <UChain/bitcoin/utility/istream_reader.hpp>
#include <UChain/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin
{
namespace chain
{

const size_t operation::max_null_data_size = 80;

operation operation::factory_from_data(const data_chunk &data)
{
    operation instance;
    instance.from_data(data);
    return instance;
}

operation operation::factory_from_data(std::istream &stream)
{
    operation instance;
    instance.from_data(stream);
    return instance;
}

operation operation::factory_from_data(reader &source)
{
    operation instance;
    instance.from_data(source);
    return instance;
}

bool operation::is_valid() const
{
    return (code == opcode::zero) && data.empty();
}

void operation::reset()
{
    code = opcode::zero;
    data.clear();
}

bool operation::from_data(const data_chunk &data)
{
    data_source istream(data);
    return from_data(istream);
}

bool operation::from_data(std::istream &stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool operation::from_data(reader &source)
{
    reset();

    const auto byte = source.read_byte();
    auto result = static_cast<bool>(source);
    auto op_code = static_cast<opcode>(byte);

    if (byte == 0 && op_code != opcode::zero)
        return false;

    code = ((0 < byte && byte <= 75) ? opcode::special : op_code);

    if (operation::must_read_data(code))
    {
        uint32_t size = 0u;
        read_opcode_data_size(size, code, byte, source);
        data = source.read_data(size);
        result = (source && (data.size() == size));
    }

    if (!result)
        reset();

    return result;
}

data_chunk operation::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void operation::to_data(std::ostream &stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void operation::to_data(writer &sink) const
{
    if (code != opcode::raw_data)
    {
        auto raw_byte = static_cast<uint8_t>(code);
        if (code == opcode::special)
            raw_byte = static_cast<uint8_t>(data.size());

        sink.write_byte(raw_byte);

        switch (code)
        {
        case opcode::pushdata1:
            sink.write_byte(static_cast<uint8_t>(data.size()));
            break;

        case opcode::pushdata2:
            sink.write_2_bytes_little_endian(
                static_cast<uint16_t>(data.size()));
            break;

        case opcode::pushdata4:
            sink.write_4_bytes_little_endian(
                static_cast<uint32_t>(data.size()));
            break;

        default:
            break;
        }
    }

    sink.write_data(data);
}

uint64_t operation::serialized_size() const
{
    uint64_t size = 1 + data.size();

    switch (code)
    {
    case opcode::pushdata1:
        size += sizeof(uint8_t);
        break;

    case opcode::pushdata2:
        size += sizeof(uint16_t);
        break;

    case opcode::pushdata4:
        size += sizeof(uint32_t);
        break;

    case opcode::raw_data:
        // remove padding for raw_data
        size -= 1;
        break;

    default:
        break;
    }

    return size;
}

std::string operation::to_string(uint32_t flags) const
{
    std::ostringstream ss;

    if (data.empty())
        ss << opcode_to_string(code, flags);
    else
        ss << "[ " << encode_base16(data) << " ]";

    return ss.str();
}

bool operation::read_opcode_data_size(uint32_t &count, opcode code,
                                      uint8_t raw_byte, reader &source)
{
    switch (code)
    {
    case opcode::special:
        count = raw_byte;
        return true;
    case opcode::pushdata1:
        count = source.read_byte();
        return true;
    case opcode::pushdata2:
        count = source.read_2_bytes_little_endian();
        return true;
    case opcode::pushdata4:
        count = source.read_4_bytes_little_endian();
        return true;
    default:
        return false;
    }
}

uint64_t operation::count_non_push(const operation::stack &ops)
{
    const auto found = [](const operation &op) {
        return !is_push(op.code);
    };

    return std::count_if(ops.begin(), ops.end(), found);
}

bool operation::must_read_data(opcode code)
{
    return code == opcode::special || code == opcode::pushdata1 || code == opcode::pushdata2 || code == opcode::pushdata4;
}

bool operation::is_push(const opcode code)
{
    return code == opcode::zero || code == opcode::special || code == opcode::pushdata1 || code == opcode::pushdata2 || code == opcode::pushdata4 || code == opcode::negative_1 || code == opcode::op_1 || code == opcode::op_2 || code == opcode::op_3 || code == opcode::op_4 || code == opcode::op_5 || code == opcode::op_6 || code == opcode::op_7 || code == opcode::op_8 || code == opcode::op_9 || code == opcode::op_10 || code == opcode::op_11 || code == opcode::op_12 || code == opcode::op_13 || code == opcode::op_14 || code == opcode::op_15 || code == opcode::op_16;
}

bool operation::is_push_only(const operation::stack &ops)
{
    return count_non_push(ops) == 0;
}

// pattern comparisons
// ----------------------------------------------------------------------------

bool operation::is_null_data_pattern(const operation::stack &ops)
{
    return ops.size() == 2 && ops[0].code == opcode::return_ && ops[1].code == opcode::special && ops[1].data.size() <= max_null_data_size;
}

bool operation::is_pay_multisig_pattern(const operation::stack &ops)
{
    static constexpr size_t op_1 = static_cast<uint8_t>(opcode::op_1);
    static constexpr size_t op_16 = static_cast<uint8_t>(opcode::op_16);

    const auto op_count = ops.size();

    if (op_count < 4 || ops[op_count - 1].code != opcode::checkmultisig)
        return false;

    const auto op_m = static_cast<uint8_t>(ops[0].code);
    const auto op_n = static_cast<uint8_t>(ops[op_count - 2].code);

    if (op_m < op_1 || op_m > op_n || op_n < op_1 || op_n > op_16)
        return false;

    //const auto n = op_n - op_1;
    const auto n = op_n - op_1 + 1u;
    const auto points = op_count - 3u;

    if (n != points)
        return false;

    for (auto op = ops.begin() + 1; op != ops.end() - 2; ++op)
        if (!is_public_key(op->data))
            return false;

    return true;
}

bool operation::is_pay_public_key_pattern(const operation::stack &ops)
{
    return ops.size() == 2 && ops[0].code == opcode::special && is_public_key(ops[0].data) && ops[1].code == opcode::checksig;
}

bool operation::is_pay_key_hash_pattern(const operation::stack &ops)
{
    return ops.size() == 5 && ops[0].code == opcode::dup && ops[1].code == opcode::hash160 && ops[2].code == opcode::special && ops[2].data.size() == short_hash_size && ops[3].code == opcode::equalverify && ops[4].code == opcode::checksig;
}

bool operation::is_pay_key_hash_with_lock_height_pattern(const operation::stack &ops)
{
    return ops.size() == 7 && ops[0].code == opcode::special && ops[1].code == opcode::numequalverify && ops[2].code == opcode::dup && ops[3].code == opcode::hash160 && ops[4].code == opcode::special && ops[4].data.size() == short_hash_size && ops[5].code == opcode::equalverify && ops[6].code == opcode::checksig;
}

bool operation::is_pay_script_hash_pattern(const operation::stack &ops)
{
    return ops.size() == 3 && ops[0].code == opcode::hash160 && ops[1].code == opcode::special && ops[1].data.size() == short_hash_size && ops[2].code == opcode::equal;
}

bool operation::is_pay_blackhole_pattern(const operation::stack &ops)
{
    return ops.size() == 1 && ops[0].code == opcode::return_;
}

bool operation::is_pay_key_hash_with_attenuation_model_pattern(const operation::stack &ops)
{
    return ops.size() == 8 && ops[0].code == opcode::pushdata2 // model param
           && ops[1].code == opcode::special                   // input point
           && ops[2].code == opcode::checkattenuationverify && ops[3].code == opcode::dup && ops[4].code == opcode::hash160 && ops[5].code == opcode::special && ops[5].data.size() == short_hash_size && ops[6].code == opcode::equalverify && ops[7].code == opcode::checksig;
}

bool operation::is_sign_multisig_pattern(const operation::stack &ops)
{
    if (ops.size() < 2 || !is_push_only(ops))
        return false;

    if (ops.front().code != opcode::zero)
        return false;

    return true;
}

bool operation::is_sign_public_key_pattern(const operation::stack &ops)
{
    return ops.size() == 1 && is_push_only(ops);
}

bool operation::is_sign_key_hash_pattern(const operation::stack &ops)
{
    return ops.size() == 2 && is_push_only(ops) &&
           is_public_key(ops.back().data);
}

uint64_t operation::get_lock_height_from_sign_key_hash_with_lock_height(const operation::stack &ops)
{
    CScriptNum num(ops[2].data, 1);
    return num.getint();
}

uint64_t operation::get_lock_height_from_pay_key_hash_with_lock_height(const operation::stack &ops)
{
    CScriptNum num(ops[0].data, 1);
    return num.getint();
}

bool operation::is_sign_key_hash_with_lock_height_pattern(const operation::stack &ops)
{
    return ops.size() == 3 && is_push_only(ops) &&
           is_public_key((ops.begin() + 1)->data);
}

bool operation::is_sign_script_hash_pattern(const operation::stack &ops)
{
    if (ops.size() < 2 || !is_push_only(ops))
        return false;

    const auto &redeem_data = ops.back().data;

    if (redeem_data.empty())
        return false;

    script redeem_script;

    if (!redeem_script.from_data(redeem_data, false, script::parse_mode::strict))
        return false;

    // Is the redeem script a standard pay (output) script?
    const auto redeem_script_pattern = redeem_script.pattern();
    return redeem_script_pattern == script_pattern::pay_multisig || redeem_script_pattern == script_pattern::pay_public_key || redeem_script_pattern == script_pattern::pay_key_hash || redeem_script_pattern == script_pattern::pay_script_hash || redeem_script_pattern == script_pattern::null_data;
}

const data_chunk &operation::get_model_param_from_pay_key_hash_with_attenuation_model(const operation::stack &ops)
{
    return ops[0].data;
}

const data_chunk &operation::get_input_point_from_pay_key_hash_with_attenuation_model(const operation::stack &ops)
{
    return ops[1].data;
}

// pattern templates
// ----------------------------------------------------------------------------

operation::stack operation::to_null_data_pattern(data_slice data)
{
    if (data.size() > max_null_data_size)
        return {};

    return operation::stack{
        {opcode::return_, {}},
        {opcode::special, to_chunk(data)}};
}

operation::stack operation::to_pay_public_key_pattern(data_slice point)
{
    if (!is_public_key(point))
        return {};

    return operation::stack{
        {opcode::special, to_chunk(point)},
        {opcode::checksig, {}}};
}

operation::stack operation::to_pay_multisig_pattern(uint8_t signatures,
                                                    const point_list &points)
{
    const auto conversion = [](const ec_compressed &point) {
        return to_chunk(point);
    };

    data_stack chunks(points.size());
    std::transform(points.begin(), points.end(), chunks.begin(), conversion);
    return to_pay_multisig_pattern(signatures, chunks);
}

operation::stack operation::to_pay_multisig_pattern(uint8_t signatures,
                                                    const data_stack &points)
{
    static constexpr size_t op_1 = static_cast<uint8_t>(opcode::op_1);
    static constexpr size_t op_16 = static_cast<uint8_t>(opcode::op_16);
    static constexpr size_t zero = op_1 - 1;
    static constexpr size_t max = op_16 - zero;

    const auto m = signatures;
    const auto n = points.size();

    if (m < 1 || m > n || n < 1 || n > max)
        return operation::stack();

    const auto op_m = static_cast<opcode>(m + zero);
    const auto op_n = static_cast<opcode>(points.size() + zero);

    operation::stack ops(points.size() + 3);
    ops.push_back({op_m, {}});

    for (const auto point : points)
    {
        if (!is_public_key(point))
            return {};

        ops.push_back({opcode::special, point});
    }

    ops.push_back({op_n, {}});
    ops.push_back({opcode::checkmultisig, {}});
    return ops;
}

operation::stack operation::to_pay_key_hash_pattern(const short_hash &hash)
{
    return operation::stack{
        {opcode::dup, {}},
        {opcode::hash160, {}},
        {opcode::special, to_chunk(hash)},
        {opcode::equalverify, {}},
        {opcode::checksig, {}}};
}

operation::stack operation::to_pay_key_hash_with_lock_height_pattern(const short_hash &hash, uint32_t block_height)
{
    return operation::stack{
        {opcode::special, CScriptNum::serialize(block_height)},
        {opcode::numequalverify, {}},
        {opcode::dup, {}},
        {opcode::hash160, {}},
        {opcode::special, to_chunk(hash)},
        {opcode::equalverify, {}},
        {opcode::checksig, {}}};
}

operation::stack operation::to_pay_script_hash_pattern(const short_hash &hash)
{
    return operation::stack{
        {opcode::hash160, {}},
        {opcode::special, to_chunk(hash)},
        {opcode::equal, {}}};
}

operation::stack operation::to_pay_blackhole_pattern(const short_hash &)
{
    return operation::stack{
        {opcode::return_, {}}};
}

operation::stack operation::to_pay_key_hash_with_attenuation_model_pattern(
    const short_hash &hash, const std::string &model_param, const point &input_point)
{
    return operation::stack{
        {opcode::pushdata2, to_chunk(model_param)},
        {opcode::special, input_point.to_data()},
        {opcode::checkattenuationverify, {}},
        {opcode::dup, {}},
        {opcode::hash160, {}},
        {opcode::special, to_chunk(hash)},
        {opcode::equalverify, {}},
        {opcode::checksig, {}}};
}

} // namespace chain
} // namespace libbitcoin
