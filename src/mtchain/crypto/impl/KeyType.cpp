//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/crypto/KeyType.h>

namespace mtchain {

KeyType keyTypeFromString (std::string const& s)
{
    if (s == "secp256k1")  return KeyType::secp256k1;
    if (s == "ed25519"  )  return KeyType::ed25519;
    if (s == "sm2p256v1")  return KeyType::sm2p256v1;

    return KeyType::invalid;
}

const char* to_string (KeyType type)
{
    switch (type)
    {
    case KeyType::secp256k1:   return "secp256k1";
    case KeyType::ed25519  :   return "ed25519";
    case KeyType::sm2p256v1:   return "sm2p256v1";
    default:                   return "INVALID";
    }
}

}
