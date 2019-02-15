/*
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS) - UChain.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#ifndef UCD_EXCEPTION_HPP
#define UCD_EXCEPTION_HPP

#include <UChainService/api/restful //exception/Exception.hpp>

/**
 * @addtogroup App
 * @{
 */

namespace mgbubble
{

class Error : public Exception
{
public:
  explicit Error(string_view what) noexcept : Exception{what} {}
  ~Error() noexcept = default;

  // Copy.
  Error(const Error &) noexcept = default;
  Error &operator=(const Error &) noexcept = default;

  // Move.
  Error(Error &&) noexcept = default;
  Error &operator=(Error &&) noexcept = default;
};

} // namespace mgbubble

/** @} */

#endif // UCD_EXCEPTION_HPP
