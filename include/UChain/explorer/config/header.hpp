/**
 * Copyright (c) 2011-2018 libbitcoin developers 
 * Copyright (c) 2018-2020 UChain core developers (check UC-AUTHORS)
 *
 * This file is part of UChain-explorer.
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
#ifndef BX_HEADER_HPP
#define BX_HEADER_HPP

#include <iostream>
#include <string>
#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin
{
namespace explorer
{
namespace config
{

/**
 * Serialization helper to convert between serialized and deserialized satoshi
 * header.
 */
class BCX_API header
{
  public:
    /**
     * Default constructor.
     */
    header();

    /**
     * Initialization constructor.
     * @param[in]  hexcode  The value to initialize with.
     */
    header(const std::string &hexcode);

    /**
     * Initialization constructor.
     * @param[in]  value  The value to initialize with.
     */
    header(const chain::header &value);

    /**
     * Copy constructor.
     * @param[in]  other  The object to copy into self on construct.
     */
    header(const header &other);

    /**
     * Overload cast to internal type.
     * @return  This object's value cast to internal type.
     */
    operator const chain::header &() const;

    /**
     * Overload stream in. Throws if input is invalid.
     * @param[in]   input     The input stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The input stream reference.
     */
    friend std::istream &operator>>(std::istream &input,
                                    header &argument);

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream &operator<<(std::ostream &output,
                                    const header &argument);

  private:
    /**
     * The state of this object's header data.
     */
    chain::header value_;
};

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#endif
