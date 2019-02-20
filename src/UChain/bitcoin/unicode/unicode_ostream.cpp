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
#include <UChain/bitcoin/unicode/unicode_ostream.hpp>

#include <cstddef>
#include <iostream>
#include <UChain/bitcoin/unicode/unicode_streambuf.hpp>

namespace libbitcoin
{

unicode_ostream::unicode_ostream(std::ostream &narrow_stream,
                                 std::wostream &wide_stream, size_t size)
#ifdef _MSC_VER
    : std::ostream(new unicode_streambuf(wide_stream.rdbuf(), size))
#else
    : std::ostream(narrow_stream.rdbuf())
#endif
{
}

unicode_ostream::~unicode_ostream()
{
#ifdef _MSC_VER
  delete rdbuf();
#endif
}

} // namespace libbitcoin
