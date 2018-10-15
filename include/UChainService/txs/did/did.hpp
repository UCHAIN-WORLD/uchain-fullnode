/**
 * Copyright (c) 2011-2018 UChain developers 
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
#ifndef UC_CHAIN_ATTACH_DID_HPP
#define UC_CHAIN_ATTACH_DID_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <UChain/bitcoin/chain/point.hpp>
#include <UChain/bitcoin/chain/script/script.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/reader.hpp>
#include <UChain/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <UChainService/txs/did/did_detail.hpp>

using namespace libbitcoin::chain;

#define DID_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<did::did_status>::type>(kd))

#define DID_DETAIL_TYPE DID_STATUS2UINT32(did::did_status::did_locked)
#define DID_TRANSFERABLE_TYPE DID_STATUS2UINT32(did::did_status::did_transferable)

namespace libbitcoin {
namespace chain {

class BC_API did
{
public:
    enum class did_status : uint32_t
    {
        did_none,
        did_locked,
        did_transferable,
    };

    did();
    did(uint32_t status, const did_detail& detail);
    static did factory_from_data(const data_chunk& data);
    static did factory_from_data(std::istream& stream);
    static did factory_from_data(reader& source);
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
    void set_data(const did_detail& detail);
    const did_detail& get_data() const;

private:
    uint32_t status;
    did_detail data;

};

} // namespace chain
} // namespace libbitcoin

#endif

