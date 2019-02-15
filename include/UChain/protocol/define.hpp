/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-protocol.
 *
 * UChain-protocol is free software: you can redistribute it and/or
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
#ifndef UC_PROTOCOL_DEFINE_HPP
#define UC_PROTOCOL_DEFINE_HPP

#include <UChain/bitcoin.hpp>

// We use the generic helper definitions in libbitcoin to define BCP_API
// and BCP_INTERNAL. BCP_API is used for the public API symbols. It either DLL
// imports or DLL exports (or does nothing for static build) BCP_INTERNAL is
// used for non-api symbols.

#if defined BCP_STATIC
#define BCP_API
#define BCP_INTERNAL
#elif defined BCP_DLL
#define BCP_API BC_HELPER_DLL_EXPORT
#define BCP_INTERNAL BC_HELPER_DLL_LOCAL
#else
#define BCP_API BC_HELPER_DLL_IMPORT
#define BCP_INTERNAL BC_HELPER_DLL_LOCAL
#endif

#if defined _WIN32
#include <winsock.h>
typedef SOCKET file_descriptor;
#else
typedef int file_descriptor;
#endif

#endif
