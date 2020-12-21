//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_PROTOCOL_H_INCLUDED
#define MTCHAIN_PROTOCOL_PROTOCOL_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <cstdint>

namespace mtchain {

/** Protocol specific constants, types, and data.

    This information is part of the MTChain protocol. Specifically,
    it is required for peers to be able to communicate with each other.

    @note Changing these will create a hard fork.

    @ingroup protocol
    @defgroup protocol
*/
struct Protocol
{
    /** Smallest legal byte size of a transaction.
    */
    static int const txMinSizeBytes = 32;

    /** Largest legal byte size of a transaction.
    */
    static int const txMaxSizeBytes = 1024 * 1024; // 1048576
};

/** A ledger index. */
using LedgerIndex = std::uint32_t;

/** A transaction identifier.
    The value is computed as the hash of the
    canonicalized, serialized transaction object.
*/
using TxID = uint256;

using TxSeq = std::uint32_t;

} //

#endif
