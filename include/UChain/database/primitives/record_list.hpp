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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UC_DATABASE_RECORD_LIST_HPP
#define UC_DATABASE_RECORD_LIST_HPP

#include <cstdint>
#include <UChain/database/define.hpp>
#include <UChain/database/memory/memory.hpp>
#include <UChain/database/primitives/record_manager.hpp>

namespace libbitcoin
{
namespace database
{

// used by test and tools only.
BC_CONSTEXPR size_t record_list_offset = sizeof(array_index);

/// This is a one-way linked list with a next value containing the index of the
/// subsequent record. Records can be dropped by forgetting an index, and
/// updating to the next value. We can think of this as a LIFO queue.
class BCD_API record_list
{
public:
  static const array_index empty;

  record_list(record_manager &manager);

  /// Create new list with a single record.
  array_index create();

  /// Insert new record before index. Returns index of new record.
  array_index insert(array_index index);

  /// Read next index for record in list.
  array_index next(array_index index) const;

  /// Get underlying record data.
  const memory_ptr get(array_index index) const;

private:
  record_manager &manager_;
};

} // namespace database
} // namespace libbitcoin

#endif
