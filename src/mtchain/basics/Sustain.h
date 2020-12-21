//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_SUSTAIN_H_INCLUDED
#define MTCHAIN_BASICS_SUSTAIN_H_INCLUDED

#include <string>

namespace mtchain {

// "Sustain" is a system for a buddy process that monitors the main process
// and relaunches it on a fault.
bool HaveSustain ();
std::string StopSustain ();
std::string DoSustain ();

} //

#endif
