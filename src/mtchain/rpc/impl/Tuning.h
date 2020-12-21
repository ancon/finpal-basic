//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_TUNING_H_INCLUDED
#define MTCHAIN_RPC_TUNING_H_INCLUDED

namespace mtchain {
namespace RPC {

/** Tuned constants. */
/** @{ */
namespace Tuning {

/** Represents RPC limit parameter values that have a min, default and max. */
struct LimitRange {
    unsigned int rmin, rdefault, rmax;
};

/** Limits for the account_lines command. */
static LimitRange const accountLines = {10, 200, 400};

/** Limits for the account_channels command. */
static LimitRange const accountChannels = {10, 200, 400};

/** Limits for the account_objects command. */
static LimitRange const accountObjects = {10, 200, 400};

/** Limits for the account_offers command. */
static LimitRange const accountOffers = {10, 200, 400};

/** Limits for the book_offers command. */
static LimitRange const bookOffers = {0, 300, 400};

/** Limits for the no_mtchain_check command. */
static LimitRange const noMtchainCheck = {10, 300, 400};

static int const defaultAutoFillFeeMultiplier = 10;
static int const defaultAutoFillFeeDivisor = 1;
static int const maxPathfindsInProgress = 2;
static int const maxPathfindJobCount = 50;
static int const maxJobQueueClients = 500;
using namespace std::chrono_literals;
auto constexpr maxValidatedLedgerAge = 60min;//2min;
static int const maxRequestSize = 1000000;

/** Maximum number of pages in one response from a binary LedgerData request. */
static int const binaryPageLength = 2048;

/** Maximum number of pages in one response from a Json LedgerData request. */
static int const jsonPageLength = 256;

/** Maximum number of pages in a LedgerData response. */
inline int pageLength(bool isBinary)
{
    return isBinary ? binaryPageLength : jsonPageLength;
}

/** Maximum number of source currencies allowed in a path find request. */
static int const max_src_cur = 18;

/** Maximum number of auto source currencies in a path find request. */
static int const max_auto_src_cur = 88;

} // Tuning
/** @} */

} // RPC
} //

#endif
