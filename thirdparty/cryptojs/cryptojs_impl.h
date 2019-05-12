/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-server.
 *
 * UChain-server is free software: you can redistribute it and/or
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
//
// Created by czp on 18-3-30.
//

#ifndef UChain_CRYPTOJS_IMPL_H
#define UChain_CRYPTOJS_IMPL_H

#include <vector>
#include <string>

namespace cryptojs
{
typedef std::vector<uint8_t> data_chunk;
data_chunk encrypt(const std::string &message, const std::string &passphrase_);

bool decrypt(const data_chunk &cipher_txt, const std::string &passphrase_, std::string &message);
} // namespace cryptojs
#endif //UChain_CRYPTJS_IMPL_H
