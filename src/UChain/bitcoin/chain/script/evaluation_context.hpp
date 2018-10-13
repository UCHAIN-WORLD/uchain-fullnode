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
#ifndef UC_CHAIN_EVALUATION_CONTEXT_HPP
#define UC_CHAIN_EVALUATION_CONTEXT_HPP

#include <algorithm>
#include <cstdint>
#include <UChain/bitcoin/chain/script/operation.hpp>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/data.hpp>
#include "conditional_stack.hpp"

namespace libbitcoin {
namespace chain {

class evaluation_context
{
public:
    data_chunk pop_stack();

    operation::stack::const_iterator code_begin;
    uint64_t operation_counter;
    data_stack stack;
    data_stack alternate;
    conditional_stack conditional;
    uint32_t flags;
};

} // namspace chain
} // namspace libbitcoin

#endif
