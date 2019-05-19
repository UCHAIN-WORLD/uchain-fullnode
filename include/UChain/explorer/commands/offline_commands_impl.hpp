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

#pragma once

#include <UChain/coin.hpp>
#include <UChain/explorer/define.hpp>
#include <UChain/explorer/utility.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

namespace bw = bc::wallet;
namespace bconfig = bc::config;

// -------------------------------------------------------------------
// macro for version
const uint32_t hd_default_secret_version = 76066276;
const uint32_t hd_default_public_version = 76067358;

// -------------------------------------------------------------------
// macro for error message
#define UCCLI_SEED_BIT_LENGTH_UNSUPPORTED \
    "The seed size is not supported."
#define UCCLI_EC_MNEMONIC_NEW_INVALID_ENTROPY \
    "The seed length in bytes is not evenly divisible by 32 bits."
#define UCCLI_EC_MNEMONIC_TO_SEED_LENGTH_INVALID_SENTENCE \
    "The number of words must be divisible by 3."
#define UCCLI_EC_MNEMONIC_TO_SEED_PASSPHRASE_UNSUPPORTED \
    "The passphrase option requires an ICU build."
#define UCCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGE \
    "The specified words are not a valid mnemonic in the specified dictionary."
#define UCCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGES \
    "WARNING: The specified words are not a valid mnemonic in any supported dictionary."
#define UCCLI_EC_MNEMONIC_TO_SEED_EMPTY_WORDS \
    "The specified words are empty."
#define UCCLI_EC_MNEMONIC_TO_SEED_WORD_NOT_IN_DICTIONARY \
    "The specified words contain illegal word that is not included in the dictionary."
#define UCCLI_HD_NEW_SHORT_SEED \
    "The seed is less than 128 bits long."
#define UCCLI_HD_NEW_INVALID_KEY \
    "The seed produced an invalid key."

// -------------------------------------------------------------------
// decalration for functions

data_chunk get_seed(uint16_t bit_length = 256u);

bw::word_list get_mnemonic_new(const bw::dictionary_list &language, const data_chunk &entropy);

data_chunk get_mnemonic_to_seed(const bw::dictionary_list &language,
                                const bw::word_list &words,
                                std::string passphrase = "");

bw::hd_private get_hd_new(const data_chunk &seed, uint32_t version = hd_default_secret_version);

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
