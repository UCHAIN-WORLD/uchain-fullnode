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
#include <UChain/coin/math/hash.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <new>
#include <stdexcept>
#include "external/crypto_scrypt.h"
#include "external/hmac_sha256.h"
#include "external/hmac_sha512.h"
#include "external/pkcs5_pbkdf2.h"
#include "external/ripemd160.h"
#include "external/sha1.h"
#include "external/sha256.h"
#include "external/sha512.h"

namespace libbitcoin
{

short_hash ripemd160_hash(data_slice data)
{
    short_hash hash;
    RMD160(data.data(), data.size(), hash.data());
    return hash;
}

short_hash sha1_hash(data_slice data)
{
    short_hash hash;
    SHA1_(data.data(), data.size(), hash.data());
    return hash;
}

hash_digest sha256_hash(data_slice data)
{
    hash_digest hash;
    SHA256_(data.data(), data.size(), hash.data());
    return hash;
}

hash_digest sha256_hash(data_slice first, data_slice second)
{
    hash_digest hash;

    SHA256CTX context;
    SHA256Init(&context);
    SHA256Update(&context, first.data(), first.size());
    SHA256Update(&context, second.data(), second.size());
    SHA256Final(&context, hash.data());

    return hash;
}

hash_digest hmac_sha256_hash(data_slice data, data_slice key)
{
    hash_digest hash;
    HMACSHA256(data.data(), data.size(), key.data(), key.size(), hash.data());
    return hash;
}

long_hash sha512_hash(data_slice data)
{
    long_hash hash;
    SHA512_(data.data(), data.size(), hash.data());
    return hash;
}

long_hash hmac_sha512_hash(data_slice data, data_slice key)
{
    long_hash hash;
    HMACSHA512(data.data(), data.size(), key.data(), key.size(), hash.data());
    return hash;
}

long_hash pkcs5_pbkdf2_hmac_sha512(data_slice passphrase,
                                   data_slice salt, size_t iterations)
{
    long_hash hash;
    const auto result = pkcs5_pbkdf2(passphrase.data(), passphrase.size(),
                                     salt.data(), salt.size(), hash.data(), hash.size(), iterations);

    if (result != 0)
        throw std::bad_alloc();

    return hash;
}

hash_digest bitcoin_hash(data_slice data)
{
    return sha256_hash(sha256_hash(data));
}

short_hash bitcoin_short_hash(data_slice data)
{
    return ripemd160_hash(sha256_hash(data));
}

static void handle_script_result(int result)
{
    if (result == 0)
        return;

    switch (errno)
    {
    case EFBIG:
        throw std::length_error("scrypt parameter too large");
    case EINVAL:
        throw std::runtime_error("scrypt invalid argument");
    case ENOMEM:
        throw std::length_error("scrypt address space");
    default:
        throw std::bad_alloc();
    }
}

data_chunk scrypt(data_slice data, data_slice salt, uint64_t N, uint32_t p,
                  uint32_t r, size_t length)
{
    data_chunk output(length);
    const auto result = crypto_scrypt(data.data(), data.size(), salt.data(),
                                      salt.size(), N, r, p, output.data(), output.size());
    handle_script_result(result);
    return output;
}

} // namespace libbitcoin
