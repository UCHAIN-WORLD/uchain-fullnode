/**
 * Copyright (c) 2011-2018 libbitcoin developers 
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <UChain/node/settings.hpp>

#include <thread>

namespace libbitcoin
{
namespace node
{

settings::settings()
    : block_timeout_seconds(5),
      download_connections(8),
      tx_pool_refresh(true)
{
}

// There are no current distinctions spanning chain contexts.
settings::settings(bc::settings context)
    : settings()
{
}

} // namespace node
} // namespace libbitcoin
