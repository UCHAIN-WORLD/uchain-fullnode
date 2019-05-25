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
#ifndef UC_ASSERT_HPP
#define UC_ASSERT_HPP

#ifdef NDEBUG
#define BITCOIN_ASSERT(expression)
#define BITCOIN_ASSERT_MSG(expression, text)
#define DEBUG_ONLY(expression)
#else
#include <cassert>
#define BITCOIN_ASSERT(expression) assert(expression)
#define BITCOIN_ASSERT_MSG(expression, text) assert((expression) && (text))
#define DEBUG_ONLY(expression) expression
#endif

// This is used to prevent bogus compiler warnings about unused variables.
// remove by -, -fno-unused
//#define UNUSED(expression) (void)(expression)

#include <UChain/coin/utility/monitor.hpp>
#include <UChain/coin/utility/track.hpp>

#endif
