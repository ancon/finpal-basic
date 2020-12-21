//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_TOKENS_H_INCLUDED
#define MTCHAIN_PROTOCOL_TOKENS_H_INCLUDED

#include <boost/optional.hpp>
#include <cstdint>
#include <string>

namespace mtchain {

enum TokenType
{
    TOKEN_NONE              = 1,
    TOKEN_NODE_PUBLIC       = 28,
    TOKEN_NODE_PRIVATE      = 32,
    TOKEN_ACCOUNT_ID        = 0,
    TOKEN_ACCOUNT_PUBLIC    = 35,
    TOKEN_ACCOUNT_SECRET    = 34,
    TOKEN_FAMILY_GENERATOR  = 41,
    TOKEN_FAMILY_SEED       = 33,
    TOKEN_SM2_SEED          = 38,
};

template <class T>
boost::optional<T>
parseBase58 (std::string const& s);

template<class T>
boost::optional<T>
parseBase58 (TokenType type, std::string const& s);

template <class T>
boost::optional<T>
parseHex (std::string const& s);

template <class T>
boost::optional<T>
parseHexOrBase58 (std::string const& s);

// Facilities for converting MTChain tokens
// to and from their human readable strings

/*  Base-58 encode a MTChain Token

    MTChain Tokens have a one-byte prefx indicating
    the type of token, followed by the data for the
    token, and finally a 4-byte checksum.

    Tokens include the following:

        Wallet Seed
        Account Public Key
        Account ID

    @param temp A pointer to storage of not
                less than 2*(size+6) bytes
*/
std::string
base58EncodeToken (std::uint8_t type,
    void const* token, std::size_t size);

/** Decode a Base58 token

    The type and checksum must match or an
    empty string is returned.
*/
std::string
decodeBase58Token(
    std::string const& s, int type);

/** Decode a Base58 token using Bitcoin alphabet

    The type and checksum must match or an
    empty string is returned.

    This is used to detect user error. Specifically,
    when an AccountID is specified using the wrong
    base58 alphabet, so that a better error message
    may be returned.
*/
std::string
decodeBase58TokenBitcoin(
    std::string const& s, int type);

} //

#endif
