//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NET_SNTPCLOCK_H_INCLUDED
#define MTCHAIN_NET_SNTPCLOCK_H_INCLUDED

#include <mtchain/beast/clock/abstract_clock.h>
#include <mtchain/beast/utility/Journal.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace mtchain {

/** A clock based on system_clock and adjusted for SNTP. */
class SNTPClock
    : public beast::abstract_clock<
        std::chrono::system_clock>
{
public:
    virtual
    void
    run (std::vector <std::string> const& servers) = 0;

    virtual
    duration
    offset() const = 0;
};

extern
std::unique_ptr<SNTPClock>
make_SNTPClock (beast::Journal);

} //

#endif
