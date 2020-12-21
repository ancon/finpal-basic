//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_TUNING_H_INCLUDED
#define MTCHAIN_RESOURCE_TUNING_H_INCLUDED

namespace mtchain {
namespace Resource {

/** Tunable constants. */
enum
{
    // Balance at which a warning is issued
     warningThreshold           = 500

    // Balance at which the consumer is disconnected
    ,dropThreshold              = 1500

    // The number of seconds until an inactive table item is removed
    ,secondsUntilExpiration     = 300

    // The number of seconds in the exponential decay window
    // (This should be a power of two)
    ,decayWindowSeconds         = 32

    // The minimum balance required in order to include a load source in gossip
    ,minimumGossipBalance       = 100

    // Number of seconds until imported gossip expires
    ,gossipExpirationSeconds    = 30
};

}
}

#endif
