//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CRYPTO_KEYTYPE_H_INCLUDED
#define MTCHAIN_CRYPTO_KEYTYPE_H_INCLUDED

#include <string>

#define MAX_SIGN_SIZE_SECP256K1  72
#define MAX_SIGN_SIZE_SM2P256V1  72
#define MAX_SIGN_SIZE_ED25519    64

namespace mtchain {

enum class KeyType
{
    invalid = -1,
    unknown = invalid,

    secp256k1 = 0,
    ed25519   = 1,
    sm2p256v1 = 2,
};

KeyType keyTypeFromString (std::string const& s);

const char* to_string (KeyType type);

template <class Stream>
inline
Stream& operator<<(Stream& s, KeyType type)
{
    return s << to_string(type);
}

}

#endif
