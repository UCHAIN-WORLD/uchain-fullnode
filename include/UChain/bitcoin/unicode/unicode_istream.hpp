﻿/**
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
#ifndef UC_UNICODE_ISTREAM_HPP
#define UC_UNICODE_ISTREAM_HPP

#include <cstddef>
#include <iostream>
#include <UChain/bitcoin/define.hpp>

namespace libbitcoin
{

/**
 * Class to expose a narrowing input stream.
 * std::wcin must be patched by console_streambuf if used for Windows input.
 */
class BC_API unicode_istream
    : public std::istream
{
  public:
    /**
     * Construct instance of a conditionally-narrowing input stream.
     * @param[in]  narrow_stream  A narrow input stream such as std::cin.
     * @param[in]  wide_stream    A wide input stream such as std::wcin.
     * @param[in]  size           The wide buffer size.
     */
    unicode_istream(std::istream &narrow_stream, std::wistream &wide_stream,
                    size_t size);

    /**
     * Delete the unicode_streambuf that wraps wide_stream.
     */
    ~unicode_istream();
};

} // namespace libbitcoin

#endif
