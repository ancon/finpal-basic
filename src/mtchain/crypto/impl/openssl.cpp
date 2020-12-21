//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/basics/contract.h>
#include <mtchain/crypto/impl/openssl.h>
#include <openssl/hmac.h>

namespace mtchain  {
namespace openssl {

bignum::bignum()
{
    ptr = BN_new();
    if (ptr == nullptr)
        Throw<std::runtime_error> ("BN_new() failed");
}

void bignum::assign (uint8_t const* data, size_t size)
{
    // This reuses and assigns ptr
    BIGNUM* bn = BN_bin2bn (data, size, ptr);
    if (bn == nullptr)
        Throw<std::runtime_error> ("BN_bin2bn() failed");
}

void bignum::assign_new (uint8_t const* data, size_t size)
{
    // ptr must not be allocated

    ptr = BN_bin2bn (data, size, nullptr);
    if (ptr == nullptr)
        Throw<std::runtime_error> ("BN_bin2bn() failed");
}

bn_ctx::bn_ctx()
{
    ptr = BN_CTX_new();
    if (ptr == nullptr)
        Throw<std::runtime_error> ("BN_CTX_new() failed");
}

bignum get_order (EC_GROUP const* group, bn_ctx& ctx)
{
    bignum result;
    if (! EC_GROUP_get_order (group, result.get(), ctx.get()))
        Throw<std::runtime_error> ("EC_GROUP_get_order() failed");

    return result;
}

ec_point::ec_point (EC_GROUP const* group)
{
    ptr = EC_POINT_new (group);
    if (ptr == nullptr)
        Throw<std::runtime_error> ("EC_POINT_new() failed");
}

void add_to (EC_GROUP const* group,
             ec_point const& a,
             ec_point& b,
             bn_ctx& ctx)
{
    if (!EC_POINT_add (group, b.get(), a.get(), b.get(), ctx.get()))
        Throw<std::runtime_error> ("EC_POINT_add() failed");
}

ec_point multiply (EC_GROUP const* group,
                   bignum const& n,
                   bn_ctx& ctx)
{
    ec_point result (group);
    if (! EC_POINT_mul (group, result.get(), n.get(), nullptr, nullptr, ctx.get()))
        Throw<std::runtime_error> ("EC_POINT_mul() failed");

    return result;
}

ec_point bn2point (EC_GROUP const* group, BIGNUM const* number)
{
    EC_POINT* result = EC_POINT_bn2point (group, number, nullptr, nullptr);
    if (result == nullptr)
        Throw<std::runtime_error> ("EC_POINT_bn2point() failed");

    return ec_point::acquire (result);
}

static ec_key ec_key_new_compressed(int NID)
{
    EC_KEY* key = EC_KEY_new_by_curve_name (NID);

    if (key == nullptr)  Throw<std::runtime_error> ("EC_KEY_new_by_curve_name() failed");

    EC_KEY_set_conv_form (key, POINT_CONVERSION_COMPRESSED);

    return ec_key((ec_key::pointer_t) key);
}

void serialize_ec_point (ec_point const& point, std::uint8_t* ptr, int NID)
{
    ec_key key = ec_key_new_compressed(NID);
    if (EC_KEY_set_public_key((EC_KEY*) key.get(), point.get()) <= 0)
        Throw<std::runtime_error> ("EC_KEY_set_public_key() failed");

    int const size = i2o_ECPublicKey ((EC_KEY*) key.get(), &ptr);

    assert (size <= 33);
    (void) size;
}

#ifndef OPENSSL_NO_SM2
#include <openssl/sm2.h>
ec_key get_ec_key_from(uint8_t const* priv_key, size_t priv_len, uint8_t const* public_key,
		       size_t public_len, int NID)
{
    ec_key key = ec_key_new_compressed(NID);

    if (priv_key != nullptr)
    {
        if (!EC_KEY_oct2priv ((EC_KEY *)key.get(), priv_key, priv_len))
        {
            Throw<std::runtime_error> ("EC_KEY_oct2priv() failed");
        }
    }

    if (public_key != nullptr)
    {
        if (!EC_KEY_oct2key((EC_KEY *)key.get(), public_key, public_len, nullptr))
        {
            Throw<std::runtime_error> ("EC_KEY_oct2key() failed");
        }
    }

    return key;
}
#endif

int sm2_sign(uint8_t const* digest, size_t len, uint8_t *sig, size_t *siglen,
             uint8_t const* priv_key, size_t priv_len)
{
#ifndef OPENSSL_NO_SM2
    ec_key key = get_ec_key_from(priv_key, priv_len, nullptr, 0, NID_sm2p256v1);

    unsigned int n = *siglen;
    int ret = SM2_sign(NID_undef, digest, len, sig, &n, (EC_KEY *)key.get());
    *siglen = n;

    return ret;
#else
    return 0;
#endif
}

int sm2_verify(uint8_t const* digest, size_t len, uint8_t const* sig, size_t siglen,
               uint8_t const* public_key, size_t public_len)
{
#ifndef OPENSSL_NO_SM2
    ec_key key = get_ec_key_from(nullptr, 0, public_key, public_len, NID_sm2p256v1);

    return SM2_verify(NID_undef, digest, len, sig, siglen, (EC_KEY *)key.get());
#else
    return 0;
#endif
}

int sm2_compute_message_digest(uint8_t const* msg, size_t msglen,
                               uint8_t const* public_key, size_t public_len,
                               uint8_t *out, size_t *poutlen,
                               const char *id, size_t idlen, const char* algo)
{
#ifndef OPENSSL_NO_SM2
    size_t digest_len = 0;

    if (!poutlen)
    {
        poutlen = &digest_len;
    }

    if (!id)
    {
        id = SM2_DEFAULT_ID;
        idlen = SM2_DEFAULT_ID_LENGTH;
    }

    if (!algo)
    {
        algo = "sm3";
        digest_len = SM3_DIGEST_LENGTH;
    }

    const EVP_MD* md = EVP_get_digestbyname(algo);
    if (!md)
    {
        return 0;
    }

    ec_key key = get_ec_key_from(nullptr, 0, public_key, public_len, NID_sm2p256v1);
    return SM2_compute_message_digest(md, md, msg, msglen, id, idlen, out, poutlen,
                                      (EC_KEY *)key.get());
#else
    return 0;
#endif
}

} // openssl
} //

#include <stdio.h>
#ifdef _MSC_VER
FILE _iob[] = {*stdin, *stdout, *stderr};
extern "C" FILE * __cdecl __iob_func(void)
{
    return _iob;
}
#endif
