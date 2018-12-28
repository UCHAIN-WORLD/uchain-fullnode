/**
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-api.
 *
 * UChain-explorer is free software: you can redistribute it and/or
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

#pragma once

#include <functional>
#include <string>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChainApp/ucd/server_node.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

bool administrator_required_checker(bc::server::server_node& node,
        const std::string& name, const std::string& auth);

uint64_t get_last_height(bc::server::server_node& node);

uint32_t get_connections_count(bc::server::server_node& node);
bool exist_in_candidates(bc::server::server_node& node, std::string uid);

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

