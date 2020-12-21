//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_SIM_PARAMS_H_INCLUDED
#define MTCHAIN_PEERFINDER_SIM_PARAMS_H_INCLUDED

namespace mtchain {
namespace PeerFinder {
namespace Sim {

/** Defines the parameters for a network simulation. */
struct Params
{
    Params ()
        : steps (50)
        , nodes (10)
        , maxPeers (20)
        , outPeers (9.5)
        , firewalled (0)
    {
    }

    int steps;
    int nodes;
    int maxPeers;
    double outPeers;
    double firewalled; // [0, 1)
};

}
}
}

#endif
