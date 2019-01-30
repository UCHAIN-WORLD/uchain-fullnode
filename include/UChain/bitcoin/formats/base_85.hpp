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
#ifndef UC_BASE_85_HPP
#define UC_BASE_85_HPP

#include <string>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/data.hpp>

namespace libbitcoin
{

/**
 * Encode data as base85 (Z85).
 * @return false if the input is not of base85 size (% 4).
 */
BC_API bool encode_base85(std::string &out, data_slice in);

/**
 * Attempt to decode base85 (Z85) data.
 * @return false if the input contains non-base85 characters or length (% 5).
 */
BC_API bool decode_base85(data_chunk &out, const std::string &in);

} // namespace libbitcoin

#endif
