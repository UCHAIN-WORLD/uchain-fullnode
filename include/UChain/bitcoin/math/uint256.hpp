// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LIBBBITCOIN_UINT256_HPP
#define LIBBBITCOIN_UINT256_HPP

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string.h>
#include <vector>
#include <UChain/bitcoin/define.hpp>
#include <UChain/bitcoin/utility/assert.hpp>

namespace libbitcoin
{

class BC_API uint_error
    : public std::runtime_error
{
  public:
    explicit uint_error(const std::string &str) : std::runtime_error(str)
    {
    }
};

/** Template base class for unsigned big integers. */
template <unsigned int BITS>
class BC_API base_uint
{
  protected:
    enum
    {
        WIDTH = BITS / 32
    };

    uint32_t pn[WIDTH];

  public:
    base_uint()
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = 0;
    }

    base_uint(const base_uint &b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];
    }

    base_uint &operator=(const base_uint &b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] = b.pn[i];

        return *this;
    }

    base_uint(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;
    }

    explicit base_uint(const std::vector<unsigned char> &vch);

    bool operator!() const
    {
        for (int i = 0; i < WIDTH; i++)
            if (pn[i] != 0)
                return false;

        return true;
    }

    const base_uint operator~() const
    {
        base_uint ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];

        return ret;
    }

    const base_uint operator-() const
    {
        base_uint ret;
        for (int i = 0; i < WIDTH; i++)
            ret.pn[i] = ~pn[i];

        ret++;
        return ret;
    }

    base_uint &operator=(uint64_t b)
    {
        pn[0] = (unsigned int)b;
        pn[1] = (unsigned int)(b >> 32);
        for (int i = 2; i < WIDTH; i++)
            pn[i] = 0;

        return *this;
    }

    base_uint &operator^=(const base_uint &b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] ^= b.pn[i];

        return *this;
    }

    base_uint &operator&=(const base_uint &b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] &= b.pn[i];

        return *this;
    }

    base_uint &operator|=(const base_uint &b)
    {
        for (int i = 0; i < WIDTH; i++)
            pn[i] |= b.pn[i];

        return *this;
    }

    base_uint &operator^=(uint64_t b)
    {
        pn[0] ^= (unsigned int)b;
        pn[1] ^= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint &operator|=(uint64_t b)
    {
        pn[0] |= (unsigned int)b;
        pn[1] |= (unsigned int)(b >> 32);
        return *this;
    }

    base_uint &operator<<=(unsigned int shift);
    base_uint &operator>>=(unsigned int shift);

    base_uint &operator+=(const base_uint &b)
    {
        uint64_t carry = 0;
        for (int i = 0; i < WIDTH; i++)
        {
            uint64_t n = carry + pn[i] + b.pn[i];
            pn[i] = n & 0xffffffff;
            carry = n >> 32;
        }

        return *this;
    }

    base_uint &operator-=(const base_uint &b)
    {
        *this += -b;
        return *this;
    }

    base_uint &operator+=(uint64_t b64)
    {
        base_uint b;
        b = b64;
        *this += b;
        return *this;
    }

    base_uint &operator-=(uint64_t b64)
    {
        base_uint b;
        b = b64;
        *this += -b;
        return *this;
    }

    base_uint &operator*=(uint32_t b32);
    base_uint &operator*=(const base_uint &b);
    base_uint &operator/=(const base_uint &b);

    base_uint &operator++()
    {
        // prefix operator
        int i = 0;
        while (++pn[i] == 0 && i < WIDTH - 1)
            i++;

        return *this;
    }

    const base_uint operator++(int)
    {
        // postfix operator
        const base_uint ret = *this;
        ++(*this);
        return ret;
    }

    base_uint &operator--()
    {
        // prefix operator
        int i = 0;
        while (--pn[i] == (uint32_t)-1 && i < WIDTH - 1)
            i++;

        return *this;
    }

    const base_uint operator--(int)
    {
        // postfix operator
        const base_uint ret = *this;
        --(*this);

        return ret;
    }

    int CompareTo(const base_uint &b) const;
    bool EqualTo(uint64_t b) const;

    friend inline const base_uint operator+(const base_uint &a, const base_uint &b) { return base_uint(a) += b; }
    friend inline const base_uint operator-(const base_uint &a, const base_uint &b) { return base_uint(a) -= b; }
    friend inline const base_uint operator*(const base_uint &a, const base_uint &b) { return base_uint(a) *= b; }
    friend inline const base_uint operator/(const base_uint &a, const base_uint &b) { return base_uint(a) /= b; }
    friend inline const base_uint operator|(const base_uint &a, const base_uint &b) { return base_uint(a) |= b; }
    friend inline const base_uint operator&(const base_uint &a, const base_uint &b) { return base_uint(a) &= b; }
    friend inline const base_uint operator^(const base_uint &a, const base_uint &b) { return base_uint(a) ^= b; }
    friend inline const base_uint operator>>(const base_uint &a, int shift) { return base_uint(a) >>= shift; }
    friend inline const base_uint operator<<(const base_uint &a, int shift) { return base_uint(a) <<= shift; }
    friend inline const base_uint operator*(const base_uint &a, uint32_t b) { return base_uint(a) *= b; }

    friend inline bool operator==(const base_uint &a, const base_uint &b) { return memcmp(a.pn, b.pn, sizeof(a.pn)) == 0; }
    friend inline bool operator!=(const base_uint &a, const base_uint &b) { return memcmp(a.pn, b.pn, sizeof(a.pn)) != 0; }
    friend inline bool operator>(const base_uint &a, const base_uint &b) { return a.CompareTo(b) > 0; }
    friend inline bool operator<(const base_uint &a, const base_uint &b) { return a.CompareTo(b) < 0; }
    friend inline bool operator>=(const base_uint &a, const base_uint &b) { return a.CompareTo(b) >= 0; }
    friend inline bool operator<=(const base_uint &a, const base_uint &b) { return a.CompareTo(b) <= 0; }
    friend inline bool operator==(const base_uint &a, uint64_t b) { return a.EqualTo(b); }
    friend inline bool operator!=(const base_uint &a, uint64_t b) { return !a.EqualTo(b); }

    unsigned char *begin()
    {
        return (unsigned char *)&pn[0];
    }

    unsigned char *end()
    {
        return (unsigned char *)&pn[WIDTH];
    }

    const unsigned char *begin() const
    {
        return (unsigned char *)&pn[0];
    }

    const unsigned char *end() const
    {
        return (unsigned char *)&pn[WIDTH];
    }

    unsigned int size() const
    {
        return sizeof(pn);
    }

    /**
     * Returns the position of the highest bit set plus one, or zero if the
     * value is zero.
     */
    unsigned int bits() const;

    uint64_t GetLow64() const
    {
        BITCOIN_ASSERT(WIDTH >= 2);
        return pn[0] | (uint64_t)pn[1] << 32;
    }
};

/** 256-bit unsigned big integer. */
class BC_API uint256_t : public base_uint<256>
{
  public:
    uint256_t()
    {
    }
    uint256_t(const base_uint<256> &b) : base_uint<256>(b)
    {
    }
    uint256_t(uint64_t b) : base_uint<256>(b)
    {
    }

    explicit uint256_t(const std::vector<unsigned char> &vch)
        : base_uint<256>(vch)
    {
    }

    /**
     * The "compact" format is a representation of a whole
     * number N using an unsigned 32bit number similar to a
     * floating point format.
     * The most significant 8 bits are the unsigned exponent of base 256.
     * This exponent can be thought of as "number of bytes of N".
     * The lower 23 bits are the mantissa.
     * Bit number 24 (0x800000) represents the sign of N.
     * N = (-1^sign) * mantissa * 256^(exponent-3)
     *
     * Satoshi's original implementation used BN_bn2mpi() and BN_mpi2bn().
     * MPI uses the most significant bit of the first byte as sign.
     * Thus 0x1234560000 is compact (0x05123456)
     * and  0xc0de000000 is compact (0x0600c0de)
     *
     * Bitcoin only uses this "compact" format for encoding difficulty
     * targets, which are unsigned 256bit quantities.  Thus, all the
     * complexities of the sign bit and using base 256 are probably an
     * implementation accident.
     */
    uint32_t GetCompact(bool fNegative = false) const;
    uint256_t &SetCompact(uint32_t nCompact, bool *pfNegative = NULL,
                          bool *pfOverflow = NULL);
};

} // namespace libbitcoin

#endif
