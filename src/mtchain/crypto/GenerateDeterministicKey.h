//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef MTCHAIN_CRYPTO_GENERATEDETERMINISTICKEY_H_INCLUDED
#define MTCHAIN_CRYPTO_GENERATEDETERMINISTICKEY_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/crypto/KeyType.h>

namespace mtchain {

Blob
generateRootDeterministicPublicKey (
    uint128 const& seed, KeyType type);

uint256
generateRootDeterministicPrivateKey (
    uint128 const& seed, KeyType type);

Blob
generatePublicDeterministicKey (
    Blob const& generator,
    int n, KeyType type);

uint256
generatePrivateDeterministicKey (
    Blob const& family,
    uint128 const& seed,
    int n, KeyType type);

} //

#endif
