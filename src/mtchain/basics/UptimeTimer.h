//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_UPTIMETIMER_H_INCLUDED
#define MTCHAIN_BASICS_UPTIMETIMER_H_INCLUDED

#include <ctime>

namespace mtchain {

/** Tracks program uptime.

    The timer can be switched to a manual system of updating, to reduce
    system calls. (?)
*/
// VFALCO TODO determine if the non-manual timing is actually needed
class UptimeTimer
{
private:
    UptimeTimer ();
    ~UptimeTimer ();

public:
    int getElapsedSeconds () const;

    void beginManualUpdates ();
    void endManualUpdates ();

    void incrementElapsedTime ();

    static UptimeTimer& getInstance ();

private:
    // VFALCO DEPRECATED, Use a memory barrier instead of forcing a cache line
    int volatile m_elapsedTime;

    time_t m_startTime;

    bool m_isUpdatingManually;
};

} //

#endif
