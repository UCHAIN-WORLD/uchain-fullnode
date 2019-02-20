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
#include <UChain/bitcoin/utility/dispatcher.hpp>

#include <string>
#include <UChain/bitcoin/utility/threadpool.hpp>
#include <UChain/bitcoin/utility/work.hpp>

namespace libbitcoin
{

dispatcher::dispatcher(threadpool &pool, const std::string &name)
    : heap_(pool, name)
{
}

size_t dispatcher::ordered_backlog()
{
    return heap_.ordered_backlog();
}

size_t dispatcher::unordered_backlog()
{
    return heap_.unordered_backlog();
}

size_t dispatcher::concurrent_backlog()
{
    return heap_.concurrent_backlog();
}

size_t dispatcher::combined_backlog()
{
    return heap_.combined_backlog();
}

} // namespace libbitcoin
