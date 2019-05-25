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
#ifndef UC_WALLET_MNEMONIC_HPP
#define UC_WALLET_MNEMONIC_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <UChain/coin/compat.hpp>
#include <UChain/coin/define.hpp>
#include <UChain/coin/math/hash.hpp>
#include <UChain/coin/unicode/unicode.hpp>
#include <UChain/coin/utility/data.hpp>
#include <UChain/coin/wallet/dictionary.hpp>

namespace libbitcoin
{
namespace wallet
{

/**
 * A valid mnemonic word count is evenly divisible by this number.
 */
static BC_CONSTEXPR size_t mnemonic_word_multiple = 3;

/**
 * A valid seed byte count is evenly divisible by this number.
 */
static BC_CONSTEXPR size_t mnemonic_seed_multiple = 4;

/**
 * Represents a mnemonic word list.
 */
typedef std::vector<std::string> word_list;

/**
 * Create a new mnenomic (list of words) from provided entropy and a dictionary
 * selection. The mnemonic can later be converted to a seed for use in wallet
 * creation. Entropy byte count must be evenly divisible by 4.
 */
BC_API word_list create_mnemonic(data_slice entropy,
                                 const dictionary &lexicon = language::en);

/**
 * Checks a mnemonic against a dictionary to determine if the
 * words are spelled correctly and the checksum matches.
 * The words must have been created using mnemonic encoding.
 */
BC_API bool validate_mnemonic(const word_list &mnemonic,
                              const dictionary &lexicon);

/**
 * Checks that a mnemonic is valid in at least one of the provided languages.
 */
BC_API bool validate_mnemonic(const word_list &mnemonic,
                              const dictionary_list &lexicons = language::all);

/**
 * Convert a mnemonic with no passphrase to a wallet-generation seed.
 */
BC_API long_hash decode_mnemonic(const word_list &mnemonic);

#ifdef WITH_ICU

/**
 * Convert a mnemonic and passphrase to a wallet-generation seed.
 * Any passphrase can be used and will change the resulting seed.
 */
BC_API long_hash decode_mnemonic(const word_list &mnemonic,
                                 const std::string &passphrase);

#endif

} // namespace wallet
} // namespace libbitcoin

#endif
