//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/crypto/GenerateDeterministicKey.h>
#include <mtchain/crypto/impl/ec_key.h>
#include <mtchain/crypto/impl/openssl.h>
#include <mtchain/protocol/digest.h>
#include <array>
#include <string>
#include <openssl/pem.h>
#include <openssl/sha.h>

namespace mtchain {

namespace openssl {

struct ECGroup
{
    EC_GROUP const* group;
    bignum order;

    ECGroup (KeyType type, bool must)
    {
        int nid = KeyType2NID (type);
        if (nid < 0)
        {
            group = nullptr;
            return;
        }

        group = EC_GROUP_new_by_curve_name (nid);
        if (!group)
        {
            if (must)
                LogicError ("The OpenSSL library on this system lacks "
                            + std::string(to_string(type)) + " support!");
            else
                LogThrow ("The OpenSSL library on this system doesn't support "
                          + std::string(to_string(type)));

            return;
        }

        bn_ctx ctx;
        order = get_order (group, ctx);
    }

    operator bool () const
    {
        return group != nullptr;
    }

    static int KeyType2NID (KeyType type)
    {
        switch (type)
        {
        case KeyType::secp256k1: return NID_secp256k1;
#ifndef OPENSSL_NO_SM2	  
        case KeyType::sm2p256v1: return NID_sm2p256v1;
#endif
        default: return -1;
        }
    }
};

static ECGroup const& getECGroup (KeyType type)
{
    static ECGroup const secp256k1curve(KeyType::secp256k1, true);
    static ECGroup const sm2p256v1curve(KeyType::sm2p256v1, false);
    static ECGroup const invalid_curve(KeyType::invalid, false);

    switch (type)
    {
    case KeyType::secp256k1: return secp256k1curve;
    case KeyType::sm2p256v1: return sm2p256v1curve;
    default: return invalid_curve;
    }
}

}  // namespace openssl

using namespace openssl;

bool isSupportKeyType (KeyType type)
{
    return (type != KeyType::ed25519) ? getECGroup (type) : true;
}

static inline ECGroup const& get_ecgroup_throw(KeyType type, const char *file, int line)
{
    auto const& G = getECGroup (type);
    if (!G)
    {
        std::stringstream os;
        os << "Unsupport key type: " << to_string(type) << " in " << file << ":" << line;
        LogicError (os.str());
    }

    return G;
}

#define get_ec_group(type)   get_ecgroup_throw(type, __FILE__, __LINE__)

static Blob serialize_ec_point (ec_point const& point, KeyType type)
{
    Blob result (33);

    serialize_ec_point (point, &result[0], ECGroup::KeyType2NID(type));

    return result;
}

template <class FwdIt>
void
copy_uint32 (FwdIt out, std::uint32_t v)
{
    *out++ =  v >> 24;
    *out++ = (v >> 16) & 0xff;
    *out++ = (v >>  8) & 0xff;
    *out   =  v        & 0xff;
}

// #define EC_DEBUG

// Functions to add support for deterministic EC keys

// --> seed
// <-- private root generator + public root generator
static bignum generateRootDeterministicKey (uint128 const& seed, ECGroup const& G)
{
    // find non-zero private key less than the curve's order
    bignum privKey;
    std::uint32_t seq = 0;

    do
    {
        // buf: 0                seed               16  seq  20
        //      |<--------------------------------->|<------>|
        std::array<std::uint8_t, 20> buf;
        std::copy(seed.begin(), seed.end(), buf.begin());
        copy_uint32 (buf.begin() + 16, seq++);
        auto root = sha512Half(buf);
        std::fill (buf.begin(), buf.end(), 0); // security erase
        privKey.assign (root.data(), root.size());
        root.zero(); // security erase
    }
    while (privKey.is_zero() || privKey >= G.order);

    return privKey;
}

// --> seed
// <-- private root generator + public root generator
Blob generateRootDeterministicPublicKey (uint128 const& seed, KeyType type)
{
    auto const& G = get_ec_group (type);

    bn_ctx ctx;
    bignum privKey = generateRootDeterministicKey (seed, G);

    // compute the corresponding public key point
    ec_point pubKey = multiply (G.group, privKey, ctx);

    privKey.clear();  // security erase

    return serialize_ec_point (pubKey, type);
}

uint256 generateRootDeterministicPrivateKey (uint128 const& seed, KeyType type)
{
    auto const& G = get_ec_group (type);
    bignum key = generateRootDeterministicKey (seed, G);

    return uint256_from_bignum_clear (key);
}

// Take mtchain address.
// --> root public generator (consumes)
// <-- root public generator in EC format
static ec_point generateRootPubKey (bignum&& pubGenerator, ECGroup const& G)
{
    ec_point pubPoint = bn2point (G.group, pubGenerator.get());

    return pubPoint;
}

// --> public generator
static bignum makeHash (Blob const& pubGen, int seq, bignum const& order)
{
    int subSeq = 0;

    bignum result;

    assert(pubGen.size() == 33);
    do
    {
        // buf: 0          pubGen             33 seq   37 subSeq  41
        //      |<--------------------------->|<------>|<-------->|
        std::array<std::uint8_t, 41> buf;
        std::copy (pubGen.begin(), pubGen.end(), buf.begin());
        copy_uint32 (buf.begin() + 33, seq);
        copy_uint32 (buf.begin() + 37, subSeq++);
        auto root = sha512Half_s(buf);
        std::fill(buf.begin(), buf.end(), 0); // security erase
        result.assign (root.data(), root.size());
    }
    while (result.is_zero()  ||  result >= order);

    return result;
}

// --> public generator
Blob generatePublicDeterministicKey (Blob const& pubGen, int seq, KeyType type)
{
    auto const& G = get_ec_group (type);

    // publicKey(n) = rootPublicKey EC_POINT_+ Hash(pubHash|seq)*point
    ec_point rootPubKey = generateRootPubKey (bignum (pubGen), G);

    bn_ctx ctx;

    // Calculate the private additional key.
    bignum hash = makeHash (pubGen, seq, G.order);

    // Calculate the corresponding public key.
    ec_point newPoint = multiply (G.group, hash, ctx);

    // Add the master public key and set.
    add_to (G.group, rootPubKey, newPoint, ctx);

    return serialize_ec_point (newPoint, type);
}

// --> root private key
uint256 generatePrivateDeterministicKey (
    Blob const& pubGen, uint128 const& seed, int seq, KeyType type)
{
    auto const& G = get_ec_group (type);

    // privateKey(n) = (rootPrivateKey + Hash(pubHash|seq)) % order
    bignum rootPrivKey = generateRootDeterministicKey (seed, G);

    bn_ctx ctx;

    // calculate the private additional key
    bignum privKey = makeHash (pubGen, seq, G.order);

    // calculate the final private key
    add_to (rootPrivKey, privKey, G.order, ctx);

    rootPrivKey.clear();  // security erase

    return uint256_from_bignum_clear (privKey);
}

} //
