//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_SECRETKEY_H_INCLUDED
#define MTCHAIN_PROTOCOL_SECRETKEY_H_INCLUDED

#include <mtchain/basics/Buffer.h>
#include <mtchain/basics/Slice.h>
#include <mtchain/crypto/KeyType.h> // move to protocol/
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/protocol/tokens.h>
#include <array>
#include <string>

namespace mtchain {

/** A secret key. */
class SecretKey
{
private:
    std::uint8_t buf_[32];

public:
    SecretKey() = default;
    SecretKey (SecretKey const&) = default;
    SecretKey& operator= (SecretKey const&) = default;

    ~SecretKey();

    SecretKey (std::array<std::uint8_t, 32> const& data);
    SecretKey (Slice const& slice);

    std::uint8_t const*
    data() const
    {
        return buf_;
    }

    std::size_t
    size() const
    {
        return sizeof(buf_);
    }

    /** Convert the secret key to a hexadecimal string.

        @note The operator<< function is deliberately omitted
        to avoid accidental exposure of secret key material.
    */
    std::string
    to_string() const;
};

inline
bool
operator== (SecretKey const& lhs,
    SecretKey const& rhs)
{
    return lhs.size() == rhs.size() &&
        std::memcmp(lhs.data(),
            rhs.data(), rhs.size()) == 0;
}

inline
bool
operator!= (SecretKey const& lhs,
    SecretKey const& rhs)
{
    return ! (lhs == rhs);
}

//------------------------------------------------------------------------------

/** Produces a sequence of secp256k1 key pairs. */
class Generator
{
private:
    Blob gen_; // VFALCO compile time size?
    KeyType type_;

public:
    explicit
    Generator (Seed const& seed, KeyType type);

    /** Generate the nth key pair.

        The seed is required to produce the private key.
    */
    std::pair<PublicKey, SecretKey>
    operator()(Seed const& seed, std::size_t ordinal) const;

    /** Generate the nth public key. */
    PublicKey
    operator()(std::size_t ordinal) const;

    operator bool() const;
};

//------------------------------------------------------------------------------

/** Parse a secret key */
template <>
boost::optional<SecretKey>
parseBase58 (TokenType type, std::string const& s);

inline
std::string
toBase58 (TokenType type, SecretKey const& sk)
{
    return base58EncodeToken(
        type, sk.data(), sk.size());
}

/** Create a secret key using secure random numbers. */
SecretKey
randomSecretKey();

/** Generate a new secret key deterministically. */
SecretKey
generateSecretKey (KeyType type, Seed const& seed);

/** Derive the public key from a secret key. */
PublicKey
derivePublicKey (KeyType type, SecretKey const& sk);

/** Generate a key pair deterministically.

    This algorithm is specific to MTChain:

    For secp256k1 key pairs, the seed is converted
    to a Generator and used to compute the key pair
    corresponding to ordinal 0 for the generator.
*/
std::pair<PublicKey, SecretKey>
generateKeyPair (KeyType type, Seed const& seed);

/** Create a key pair using secure random numbers. */
std::pair<PublicKey, SecretKey>
randomKeyPair (KeyType type);

/** Generate a signature for a message digest.
    This can only be used with secp256k1 since Ed25519's
    security properties come, in part, from how the message
    is hashed.
*/
/** @{ */
Buffer
signDigest (PublicKey const& pk, SecretKey const& sk,
    uint256 const& digest);

inline
Buffer
signDigest (KeyType type, SecretKey const& sk,
    uint256 const& digest)
{
    return signDigest (derivePublicKey(type, sk), sk, digest);
}
/** @} */

/** Generate a signature for a message.
    With secp256k1 signatures, the data is first hashed with
    SHA512-Half, and the resulting digest is signed.
*/
/** @{ */
Buffer
sign (PublicKey const& pk,
    SecretKey const& sk, Slice const& message);

inline
Buffer
sign (KeyType type, SecretKey const& sk,
    Slice const& message)
{
    return sign (derivePublicKey(type, sk), sk, message);
}
/** @} */

bool isSupportKeyType (KeyType type);

} //

#endif
