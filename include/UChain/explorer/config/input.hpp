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
#ifndef BX_INPUT_HPP
#define BX_INPUT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <UChain/bitcoin.hpp>
#include <UChain/explorer/define.hpp>

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin
{
namespace explorer
{
namespace config
{

/**
 * Serialization helper stub for tx_input_type.
 */
class BCX_API input
{
  public:
    /**
     * Default constructor.
     */
    input();

    /**
     * Initialization constructor.
     * @param[in]  tuple  The value to initialize with.
     */
    input(const std::string &tuple);

    /**
     * Initialization constructor. Only the point is retained.
     * @param[in]  value  The value to initialize with.
     */
    input(const tx_input_type &value);

    /**
     * Copy constructor.
     * @param[in]  other  The object to copy into self on construct.
     */
    input(const input &other);

    /**
     * Initialization constructor. Aspects of the input other than the point
     * are defaulted.
     * @param[in]  value  The value to initialize with.
     */
    input(const chain::input_point &value);

    /**
     * Overload cast to internal type.
     * @return  This object's value cast to internal type.
     */
    operator const tx_input_type &() const;

    /**
     * Overload stream in. Throws if input is invalid.
     * @param[in]   input     The input stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The input stream reference.
     */
    friend std::istream &operator>>(std::istream &stream,
                                    input &argument);

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream &operator<<(std::ostream &output,
                                    const input &argument);

  private:
    /**
     * The state of this object.
     */
    tx_input_type value_;
};

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#endif
