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
#ifndef UC_CHAIN_CONDITIONAL_STACK_HPP
#define UC_CHAIN_CONDITIONAL_STACK_HPP

#include <vector>

namespace libbitcoin
{
namespace chain
{

class conditional_stack
{
  public:
    bool closed() const;
    bool succeeded() const;
    void clear();
    void open(bool value);
    void else_();
    void close();

  private:
    std::vector<bool> stack_;
};

} // namespace chain
} // namespace libbitcoin

#endif
