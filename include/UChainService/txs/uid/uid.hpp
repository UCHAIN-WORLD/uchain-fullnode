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
#ifndef UC_CHAIN_ATTACH_UID_HPP
#define UC_CHAIN_ATTACH_UID_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <UChainService/txs/uid/uid_detail.hpp>

using namespace libbitcoin::chain;

#define UID_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<uid::uid_status>::type>(kd))

#define UID_DETAIL_TYPE UID_STATUS2UINT32(uid::uid_status::uid_locked)
#define UID_TRANSFERABLE_TYPE UID_STATUS2UINT32(uid::uid_status::uid_transferable)

namespace libbitcoin {
namespace chain {

class BC_API uid
{
public:
    enum class uid_status : uint32_t
    {
        uid_none,
        uid_locked,
        uid_transferable,
    };

    uid();
    uid(uint32_t status, const uid_detail& detail);
    static uid factory_from_data(const data_chunk& data);
    static uid factory_from_data(std::istream& stream);
    static uid factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    std::string to_string() const;
    bool is_valid_type() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    uint32_t get_status() const;
    void set_status(uint32_t status);
    void set_data(const uid_detail& detail);
    const uid_detail& get_data() const;

private:
    uint32_t status;
    uid_detail data;

};

} // namespace chain
} // namespace libbitcoin

#endif

