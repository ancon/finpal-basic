//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_OVERLAY_TUNING_H_INCLUDED
#define MTCHAIN_OVERLAY_TUNING_H_INCLUDED

namespace mtchain {

namespace Tuning
{

enum
{
    /** Size of buffer used to read from the socket. */
    readBufferBytes     = 4096,

    /** How long a server can remain insane before we
        disconnected it (if outbound) */
    maxInsaneTime       =   60,

    /** How long a server can remain unknown before we
        disconnect it (if outbound) */
    maxUnknownTime      =  300,

    /** How many ledgers off a server can be and we will
        still consider it sane */
    saneLedgerLimit     =   24,

    /** How many ledgers off a server has to be before we
        consider it insane */
    insaneLedgerLimit   =  128,

    /** The maximum number of ledger entries in a single
        reply */
    maxReplyNodes       = 8192,

    /** How many milliseconds to consider high latency
        on a peer connection */
    peerHighLatency     =  250,

    /** How often we check connections (seconds) */
    checkSeconds        =   10,

    /** How often we latency/sendq probe connections */
    timerSeconds        =    4,

    /** How many timer intervals a sendq has to stay large before we disconnect */
    sendqIntervals      =    4,

    /** How many timer intervals we can go without a ping reply */
    noPing              =   10,

    /** How many messages on a send queue before we refuse queries */
    dropSendQueue       =    8,

    /** How many messages we consider reasonable sustained on a send queue */
    targetSendQueue     =   16,
};

} // Tuning

} //

#endif
