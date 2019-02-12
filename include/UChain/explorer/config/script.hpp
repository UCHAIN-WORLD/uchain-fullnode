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
#ifndef BX_SCRIPT_HPP
#define BX_SCRIPT_HPP

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
 * Serialization helper to convert between base16/raw script and script_type.
 */
class BCX_API script
{
  public:
    /**
     * Default constructor.
     */
    script();

    /**
     * Initialization constructor.
     * @param[in]  mnemonic  The value to initialize with.
     */
    script(const std::string &mnemonic);

    /**
     * Initialization constructor.
     * @param[in]  value  The value to initialize with.
     */
    script(const chain::script &value);

    /**
     * Initialization constructor.
     * @param[in]  value  The value to initialize with.
     */
    script(const data_chunk &value);

    /**
     * Initialization constructor.
     * @param[in]  tokens  The mnemonic tokens to initialize with.
     */
    script(const std::vector<std::string> &tokens);

    /**
     * Copy constructor.
     * @param[in]  other  The object to copy into self on construct.
     */
    script(const script &other);

    /**
     * Serialize the script to bytes according to the wire protocol.
     * @return  The byte serialized copy of the script.
     */
    const bc::data_chunk to_data() const;

    /**
     * Return a pretty-printed copy of the script.
     * @return  A mnemonic-printed copy of the internal script.
     */
    const std::string to_string() const;

    /**
     * Overload cast to internal type.
     * @return  This object's value cast to internal type.
     */
    operator const chain::script &() const;

    /**
     * Overload stream in. Throws if input is invalid.
     * @param[in]   input     The input stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The input stream reference.
     */
    friend std::istream &operator>>(std::istream &input,
                                    script &argument);

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream &operator<<(std::ostream &output,
                                    const script &argument);

  private:
    /**
     * The state of this object.
     */
    chain::script value_;
};

} // namespace config
} // namespace explorer
} // namespace libbitcoin

#endif
