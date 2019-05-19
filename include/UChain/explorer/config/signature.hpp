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
#ifndef BX_SIGNATURE_HPP
#define BX_SIGNATURE_HPP

#include <array>
#include <iostream>
#include <string>
#include <cstdint>
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
 * Serialization helper to convert between string and message_signature.
 */
class BCX_API signature
{
  public:
    /**
     * Default constructor.
     */
    signature();

    /**
     * Initialization constructor.
     * @param[in]  hexcode  The value to initialize with.
     */
    signature(const std::string &hexcode);

    /**
     * Initialization constructor.
     * @param[in]  value  The value to initialize with.
     */
    signature(const bc::wallet::message_signature &value);

    /**
     * Copy constructor.
     * @param[in]  other  The object to copy into self on construct.
     */
    signature(const signature &other);

    /**
     * Overload cast to internal type.
     * @return  This object's value cast to internal type.
     */
    operator const bc::wallet::message_signature &() const;

    /**
     * Overload stream in. If input is invalid sets no bytes in argument.
     * @param[in]   input     The input stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The input stream reference.
     */
    friend std::istream &operator>>(std::istream &input,
                                    signature &argument);

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream &operator<<(std::ostream &output,
                                    const signature &argument);

  private:
    /**
     * The state of this object.
     */
    bc::wallet::message_signature value_;
};

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#endif
