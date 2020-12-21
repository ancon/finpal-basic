//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_PRIVATETYPES_H_INCLUDED
#define MTCHAIN_PEERFINDER_PRIVATETYPES_H_INCLUDED

namespace mtchain {
namespace PeerFinder {

/** Indicates the action the logic will take after a handshake. */
enum HandshakeAction
{
    doActivate,
    doRedirect,
    doClose
};

}
}

#endif
