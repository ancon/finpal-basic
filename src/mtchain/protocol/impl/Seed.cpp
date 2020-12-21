//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/Buffer.h>
#include <mtchain/protocol/AccountID.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/basics/contract.h>
#include <mtchain/crypto/RFC1751.h>
#include <mtchain/crypto/csprng.h>
#include <mtchain/beast/crypto/secure_erase.h>
#include <mtchain/beast/utility/rngfill.h>
#include <algorithm>
#include <cstring>
#include <iterator>

namespace mtchain {

Seed::~Seed()
{
    beast::secure_erase(buf_.data(), buf_.size());
}

Seed::Seed (Slice const& slice, KeyType type)
{
    if (slice.size() != buf_.size())
        LogicError("Seed::Seed: invalid size");
    std::memcpy(buf_.data(),
        slice.data(), buf_.size());
    type_ = type;
}

Seed::Seed (uint128 const& seed, KeyType type)
{
    if (seed.size() != buf_.size())
        LogicError("Seed::Seed: invalid size");
    std::memcpy(buf_.data(),
        seed.data(), buf_.size());
    type_ = type;
}

std::string toBase58 (Seed const& seed)
{
    switch (seed.type())
    {
    default:
    case KeyType::secp256k1:
        return base58EncodeToken(TokenType::TOKEN_FAMILY_SEED, seed.data(), seed.size());
    case KeyType::sm2p256v1:
        return base58EncodeToken(TokenType::TOKEN_SM2_SEED, seed.data(), seed.size());
    case KeyType::ed25519:
    {
        Blob blob;

        blob.push_back (0xE1);
        blob.push_back (0x4B);
        blob.insert (blob.end(), seed.data(), seed.data() + seed.size());
        return base58EncodeToken(TokenType::TOKEN_NONE, blob.data(), blob.size());
    }
    }
}

//------------------------------------------------------------------------------

Seed
randomSeed()
{
    std::array <std::uint8_t, 16> buffer;
    beast::rngfill (
        buffer.data(),
        buffer.size(),
        crypto_prng());
    Seed seed (makeSlice (buffer));
    beast::secure_erase(buffer.data(), buffer.size());
    return seed;
}

Seed
generateSeed (std::string const& passPhrase)
{
    sha512_half_hasher_s h;
    h(passPhrase.data(), passPhrase.size());
    auto const digest =
        sha512_half_hasher::result_type(h);
    return Seed({ digest.data(), 16 });
}

template <>
boost::optional<Seed>
parseBase58 (std::string const& s)
{
    auto result = decodeBase58Token(s, TokenType::TOKEN_FAMILY_SEED);
    if (result.size() == 16)
    {
        return Seed(makeSlice(result), KeyType::secp256k1);
    }

    result = decodeBase58Token(s, TokenType::TOKEN_SM2_SEED);
    if (result.size() == 16)
    {
        return Seed(makeSlice(result), KeyType::sm2p256v1);
    }

    result = decodeBase58Token(s, TokenType::TOKEN_NONE);
    if (result.size() == 18 && result[0] == 0xE1 && result[1] == 0x4B)
    {
        return Seed(makeSlice(result.erase(0, 2)), KeyType::ed25519);
    }

    return boost::none;
}

boost::optional<Seed>
parseGenericSeed (std::string const& str)
{
    if (str.empty ())
        return boost::none;

    if (parseBase58<AccountID>(str) ||
        parseBase58<PublicKey>(TokenType::TOKEN_NODE_PUBLIC, str) ||
        parseBase58<PublicKey>(TokenType::TOKEN_ACCOUNT_PUBLIC, str) ||
        parseBase58<SecretKey>(TokenType::TOKEN_NODE_PRIVATE, str) ||
        parseBase58<SecretKey>(TokenType::TOKEN_ACCOUNT_SECRET, str))
    {
        return boost::none;
    }

    {
        uint128 seed;

        if (seed.SetHexExact (str))
            return Seed { Slice(seed.data(), seed.size()) };
    }

    if (auto seed = parseBase58<Seed> (str))
        return seed;

    {
        std::string key;
        if (RFC1751::getKeyFromEnglish (key, str) == 1)
        {
            Blob const blob (key.rbegin(), key.rend());
            return Seed{ uint128{blob} };
        }
    }

    return generateSeed (str);
}

std::string
seedAs1751 (Seed const& seed)
{
    std::string key;

    std::reverse_copy (
        seed.data(),
        seed.data() + 16,
        std::back_inserter(key));

    std::string encodedKey;
    RFC1751::getEnglishFromKey (encodedKey, key);
    return encodedKey;
}

}
