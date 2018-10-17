/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (see UC-AUTHORS)
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
#ifndef UC_VARIABLE_UINT_SIZE_HPP
#define UC_VARIABLE_UINT_SIZE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <UChain/bitcoin/define.hpp>

namespace libbitcoin {

BC_API size_t variable_uint_size(uint64_t value);
BC_API size_t variable_string_size(const std::string& str);
BC_API std::string limit_size_string(const std::string& str, size_t limit_size);

} // namespace libbitcoin

#endif

