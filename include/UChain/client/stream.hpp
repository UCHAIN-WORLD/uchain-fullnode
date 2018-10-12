/**
 * Copyright (c) 2011-2018 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
 *
 * This file is part of UChain-client.
 *
 * UChain-client is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef UC_CLIENT_STREAM_HPP
#define UC_CLIENT_STREAM_HPP

#include <cstdint>
#include <UChain/bitcoin.hpp>
#include <UChain/client/define.hpp>

namespace libbitcoin {
namespace client {

/// A stream of multi-part messages.
/// This interface breaks the link between zeromq networking and messaging.
class BCC_API stream
{
public:

    /// Resend any timed out work and return the smallest time remaining.
    virtual int32_t refresh() = 0;

    /// Read from this stream onto the specified stream.
    virtual bool read(stream& stream) = 0;

    /// Write the specified data to this stream.
    virtual bool write(const data_stack& data) = 0;
};

} // namespace client
} // namespace libbitcoin

#endif
