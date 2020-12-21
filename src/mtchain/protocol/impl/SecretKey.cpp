//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/protocol/impl/secp256k1.h>
#include <mtchain/basics/contract.h>
#include <mtchain/crypto/GenerateDeterministicKey.h>
#include <mtchain/crypto/csprng.h>
#include <mtchain/beast/crypto/secure_erase.h>
#include <mtchain/beast/utility/rngfill.h>
#include <ed25519-donna/ed25519.h>
#include <cstring>
#include <mtchain/crypto/impl/openssl.h>

namespace mtchain {

SecretKey::~SecretKey()
{
    beast::secure_erase(buf_, sizeof(buf_));
}

SecretKey::SecretKey (std::array<std::uint8_t, 32> const& key)
{
    std::memcpy(buf_, key.data(), key.size());
}

SecretKey::SecretKey (Slice const& slice)
{
    if (slice.size() != sizeof(buf_))
        LogicError("SecretKey::SecretKey: invalid size");
    std::memcpy(buf_, slice.data(), sizeof(buf_));
}

std::string
SecretKey::to_string() const
{
    return strHex(data(), size());
}

//------------------------------------------------------------------------------

Generator::Generator (Seed const& seed, KeyType type)
{
    type_ = type;
    if (!isSupportKeyType(type)) return;

    uint128 ui;
    std::memcpy(ui.data(), seed.data(), seed.size());

    gen_ = generateRootDeterministicPublicKey(ui, type);
}

std::pair<PublicKey, SecretKey>
Generator::operator()(Seed const& seed, std::size_t ordinal) const
{
    uint128 ui;
    std::memcpy(ui.data(), seed.data(), seed.size());
    auto gsk = generatePrivateDeterministicKey(gen_, ui, ordinal, type_);
    auto gpk = generatePublicDeterministicKey(gen_, ordinal, type_);
    if (type_ == KeyType::sm2p256v1)
    {
        assert (gpk[0] == 0x02 || gpk[0] == 0x03);
        gpk[0] |= 0x20;
    }
    SecretKey const sk(Slice{ gsk.data(), gsk.size() });
    PublicKey const pk(Slice{ gpk.data(), gpk.size() });
    beast::secure_erase(ui.data(), ui.size());
    beast::secure_erase(gsk.data(), gsk.size());
    return { pk, sk };
}

PublicKey
Generator::operator()(std::size_t ordinal) const
{
    auto gpk = generatePublicDeterministicKey(gen_, ordinal, type_);
    return PublicKey(Slice{ gpk.data(), gpk.size() });
}


Generator::operator bool() const
{
    return isSupportKeyType(type_);
}

//------------------------------------------------------------------------------

Buffer
signDigest (PublicKey const& pk, SecretKey const& sk,
    uint256 const& digest)
{
    if (publicKeyType(pk.slice()) != KeyType::secp256k1)
        LogicError("sign: secp256k1 required for digest signing");

    BOOST_ASSERT(sk.size() == 32);
    secp256k1_ecdsa_signature sig_imp;
    if(secp256k1_ecdsa_sign(
            secp256k1Context(),
            &sig_imp,
            reinterpret_cast<unsigned char const*>(
                digest.data()),
            reinterpret_cast<unsigned char const*>(
                sk.data()),
            secp256k1_nonce_function_rfc6979,
            nullptr) != 1)
        LogicError("sign: secp256k1_ecdsa_sign failed");

    unsigned char sig[72];
    size_t len = sizeof(sig);
    if(secp256k1_ecdsa_signature_serialize_der(
            secp256k1Context(),
            sig,
            &len,
            &sig_imp) != 1)
        LogicError("sign: secp256k1_ecdsa_signature_serialize_der failed");

    return Buffer{sig, len};
}

Buffer
sign (PublicKey const& pk,
    SecretKey const& sk, Slice const& m)
{
    auto const type =
        publicKeyType(pk.slice());
    if (! type)
        LogicError("sign: invalid type");
    switch(*type)
    {
    case KeyType::secp256k1:
    {
        sha512_half_hasher h;
        h(m.data(), m.size());
        auto const digest =
            sha512_half_hasher::result_type(h);

        secp256k1_ecdsa_signature sig_imp;
        if(secp256k1_ecdsa_sign(
                secp256k1Context(),
                &sig_imp,
                reinterpret_cast<unsigned char const*>(
                    digest.data()),
                reinterpret_cast<unsigned char const*>(
                    sk.data()),
                secp256k1_nonce_function_rfc6979,
                nullptr) != 1)
            LogicError("sign: secp256k1_ecdsa_sign failed");

        unsigned char sig[MAX_SIGN_SIZE_SECP256K1];
        size_t len = sizeof(sig);
        if(secp256k1_ecdsa_signature_serialize_der(
                secp256k1Context(),
                sig,
                &len,
                &sig_imp) != 1)
            LogicError("sign: secp256k1_ecdsa_signature_serialize_der failed");

        return Buffer{sig, len};
    }
    case KeyType::sm2p256v1:
    {
        //std::uint8_t public_key[MAX_PUBLIC_KEY_SIZE];
        //memcpy(public_key, pk.data(), pk.size());
        Blob public_key(pk.data(), pk.data() + pk.size());
        assert(public_key[0] == 0x22 || public_key[0] == 0x23);
        public_key[0] &= 0xF;

        //auto const digest = sha512Half(m);
        std::uint8_t buf[SM3_DIGEST_LENGTH] = { 0 };
        if (!openssl::sm2_compute_message_digest(m.data(), m.size(), public_key.data(),
                                                 public_key.size(), buf))
            LogicError("sign: sm2_compute_messge_digest failed");

        Slice digest(buf, sizeof(buf));
        unsigned char sig[MAX_SIGN_SIZE_SM2P256V1];
        size_t len = sizeof(sig);
        if (!openssl::sm2_sign(digest.data(), digest.size(), sig, &len, sk.data(), sk.size()))
            LogicError("sign: sm2_sign failed");

        return Buffer{sig, len};
    }
    case KeyType::ed25519:
    {
        Buffer b(MAX_SIGN_SIZE_ED25519);
        ed25519_sign(m.data(), m.size(),
            sk.data(), pk.data() + 1, b.data());
        return b;
    }
    default:
        LogicError("sign: invalid type");
    }
}

SecretKey
randomSecretKey()
{
    std::uint8_t buf[32];
    beast::rngfill(
        buf,
        sizeof(buf),
        crypto_prng());
    SecretKey sk(Slice{ buf, sizeof(buf) });
    beast::secure_erase(buf, sizeof(buf));
    return sk;
}

// VFALCO TODO Rewrite all this without using OpenSSL
//             or calling into GenerateDetermisticKey
SecretKey
generateSecretKey (KeyType type, Seed const& seed)
{
    if (type == KeyType::ed25519)
    {
        auto const key = sha512Half_s(Slice(
            seed.data(), seed.size()));
        return SecretKey(Slice{ key.data(), key.size() });
    }

    if (type == KeyType::secp256k1)
    {
        uint128 ps;
        std::memcpy(ps.data(),
            seed.data(), seed.size());
        auto const upk =
            generateRootDeterministicPrivateKey(ps, type);
        return SecretKey(Slice{ upk.data(), upk.size() });
    }

    LogicError ("generateSecretKey: unknown key type");
}

PublicKey
derivePublicKey (KeyType type, SecretKey const& sk)
{
    switch(type)
    {
    case KeyType::secp256k1:
    {
        secp256k1_pubkey pubkey_imp;
        if(secp256k1_ec_pubkey_create(
                secp256k1Context(),
                &pubkey_imp,
                reinterpret_cast<unsigned char const*>(
                    sk.data())) != 1)
            LogicError("derivePublicKey: secp256k1_ec_pubkey_create failed");

        unsigned char pubkey[33];
        size_t len = sizeof(pubkey);
        if(secp256k1_ec_pubkey_serialize(
                secp256k1Context(),
                pubkey,
                &len,
                &pubkey_imp,
                SECP256K1_EC_COMPRESSED) != 1)
            LogicError("derivePublicKey: secp256k1_ec_pubkey_serialize failed");

        return PublicKey{Slice{pubkey,
            static_cast<std::size_t>(len)}};
    }
    case KeyType::ed25519:
    {
        unsigned char buf[33];
        buf[0] = 0xED;
        ed25519_publickey(sk.data(), &buf[1]);
        return PublicKey(Slice{ buf, sizeof(buf) });
    }
    default:
        LogicError("derivePublicKey: bad key type");
    };
}

std::pair<PublicKey, SecretKey>
generateKeyPair (KeyType type, Seed const& seed)
{
    assert (seed.type() == KeyType::unknown || seed.type() == type);
    switch(type)
    {
    default: type = KeyType::secp256k1;
    case KeyType::secp256k1:
    case KeyType::sm2p256v1:
    {
        Generator g(seed, type);
        return g(seed, 0);
    }
    case KeyType::ed25519:
    {
        auto const sk = generateSecretKey(type, seed);
        return { derivePublicKey(type, sk), sk };
    }
    }
}

std::pair<PublicKey, SecretKey>
randomKeyPair (KeyType type)
{
    auto const sk = randomSecretKey();
    return { derivePublicKey(type, sk), sk };
}

template <>
boost::optional<SecretKey>
parseBase58 (TokenType type, std::string const& s)
{
    auto const result = decodeBase58Token(s, type);
    if (result.empty())
        return boost::none;
    if (result.size() != 32)
        return boost::none;
    return SecretKey(makeSlice(result));
}

} //

