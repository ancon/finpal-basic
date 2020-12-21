//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_REPORTING_H_INCLUDED
#define MTCHAIN_PEERFINDER_REPORTING_H_INCLUDED

namespace mtchain {
namespace PeerFinder {

/** Severity levels for test reporting.
    This allows more fine grained control over reporting for diagnostics.
*/
struct Reporting
{
    // Report simulation parameters
    static bool const params = true;

    // Report simulation crawl time-evolution
    static bool const crawl = true;

    // Report nodes aggregate statistics
    static bool const nodes = true;

    // Report nodes detailed information
    static bool const dump_nodes = false;
};

}
}

#endif
