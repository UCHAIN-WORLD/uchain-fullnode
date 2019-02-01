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
#ifndef UC_WALLET_DICTIONARY_HPP
#define UC_WALLET_DICTIONARY_HPP

#include <array>
#include <vector>
#include <UChain/bitcoin/compat.hpp>

namespace libbitcoin
{
namespace wallet
{

/**
 * A valid mnemonic dictionary has exactly this many words.
 */
static BC_CONSTEXPR size_t dictionary_size = 2048;

/**
 * A dictionary for creating mnemonics.
 * The bip39 spec calls this a "wordlist".
 * This is a POD type, which means the compiler can write it directly
 * to static memory with no run-time overhead.
 */
typedef std::array<const char *, dictionary_size> dictionary;

/**
 * A collection of candidate dictionaries for mnemonic validation.
 */
typedef std::vector<const dictionary *> dictionary_list;

namespace language
{
// Individual built-in languages:
extern const dictionary en;
extern const dictionary es;
extern const dictionary ja;
extern const dictionary zh_Hans;
extern const dictionary zh_Hant;

// All built-in languages:
extern const dictionary_list all;
} // namespace language

namespace symbol
{
// built in ban dict (upper case):
bool is_sensitive(const std::string &symbol);

// built in forbidden dict (upper case):
bool is_forbidden(const std::string &symbol);

// All built-in ban list:
} // namespace symbol

} // namespace wallet
} // namespace libbitcoin

#endif
