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
#ifndef UC_DATABASE_RECORD_MULTIMAP_ITERATOR_HPP
#define UC_DATABASE_RECORD_MULTIMAP_ITERATOR_HPP

#include <UChain/database/define.hpp>
#include <UChain/database/primitives/record_list.hpp>

namespace libbitcoin
{
namespace database
{

/// Forward iterator for multimap record values.
/// After performing key lookup iterate the multiple values in a for loop.
class BCD_API record_multimap_iterator
{
  public:
    record_multimap_iterator(const record_list &records, array_index index);

    /// Next value in the chain.
    void operator++();

    /// The record index.
    array_index operator*() const;

    /// Comparison operators.
    bool operator==(record_multimap_iterator other) const;
    bool operator!=(record_multimap_iterator other) const;

  private:
    array_index index_;
    const record_list &records_;
};

} // namespace database
} // namespace libbitcoin

#endif
