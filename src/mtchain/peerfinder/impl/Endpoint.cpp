//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/peerfinder/PeerfinderManager.h>

namespace mtchain {
namespace PeerFinder {

Endpoint::Endpoint ()
    : hops (0)
{
}

Endpoint::Endpoint (beast::IP::Endpoint const& ep, int hops_)
    : hops (hops_)
    , address (ep)
{
}

bool operator< (Endpoint const& lhs, Endpoint const& rhs)
{
    return lhs.address < rhs.address;
}

}
}
