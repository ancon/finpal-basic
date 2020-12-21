//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_SEED_H_INCLUDED
#define MTCHAIN_PROTOCOL_SEED_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/basics/Slice.h>
#include <mtchain/protocol/tokens.h>
#include <mtchain/crypto/KeyType.h>
#include <boost/optional.hpp>
#include <array>

namespace mtchain {

/** Seeds are used to generate deterministic secret keys. */
class Seed
{
private:
    std::array<uint8_t, 16> buf_;
    KeyType type_;

public:
    Seed() = delete;

    Seed (Seed const&) = default;
    Seed& operator= (Seed const&) = default;

    /** Destroy the seed.
        The buffer will first be securely erased.
    */
    ~Seed();

    /** Construct a seed */
    /** @{ */
    explicit Seed (Slice   const& slice, KeyType type = KeyType::unknown);
    explicit Seed (uint128 const& seed,  KeyType type = KeyType::unknown);
    /** @} */

    std::uint8_t const*
    data() const
    {
        return buf_.data();
    }

    std::size_t
    size() const
    {
        return buf_.size();
    }

    KeyType type() const
    {
        return type_;
    }

    void setType (KeyType type)
    {
        type_ = type;
    }
};

//------------------------------------------------------------------------------

/** Create a seed using secure random numbers. */
Seed
randomSeed();

/** Generate a seed deterministically.

    The algorithm is specific to MTChain:

        The seed is calculated as the first 128 bits
        of the SHA512-Half of the string text excluding
        any terminating null.

    @note This will not attempt to determine the format of
          the string (e.g. hex or base58).
*/
Seed
generateSeed (std::string const& passPhrase);

/** Parse a Base58 encoded string into a seed */
template <>
boost::optional<Seed>
parseBase58 (std::string const& s);

/** Attempt to parse a string as a seed */
boost::optional<Seed>
parseGenericSeed (std::string const& str);

/** Encode a Seed in RFC1751 format */
std::string
seedAs1751 (Seed const& seed);

/** Format a seed as a Base58 string */
std::string toBase58 (Seed const& seed);

}

#endif
