/**
 * Copyright (c) 2011-2018 uc developers
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
#include <memory>
#include <string>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/command.hpp>

namespace libbitcoin
{
namespace explorer
{

std::string formerly_extension(const std::string &former);

std::shared_ptr<command> find_extension(const std::string &symbol);

void broadcast_extension(const std::function<void(std::shared_ptr<command>)> func, std::ostream &os);
bool check_read_only(const string &symbol);

} // namespace explorer
} // namespace libbitcoin
