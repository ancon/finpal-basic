//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_CONSENSUS_RCLCXTRAITS_H_INCLUDED
#define MTCHAIN_APP_CONSENSUS_RCLCXTRAITS_H_INCLUDED

#include <mtchain/basics/chrono.h>
#include <mtchain/basics/base_uint.h>

#include <mtchain/protocol/UintTypes.h>
#include <mtchain/protocol/MTChainLedgerHash.h>

#include <mtchain/app/consensus/RCLCxPos.h>
#include <mtchain/app/consensus/RCLCxTx.h>

namespace mtchain {

// Consensus traits class
// For adapting consensus to RCL

class RCLCxTraits
{
public:

    using Time_t    = NetClock::time_point;

    using Pos_t     = RCLCxPos;
    using TxSet_t   = RCLTxSet;
    using Tx_t      = RCLCxTx;

    using LgrID_t   = LedgerHash;
    using TxID_t    = uint256;
    using TxSetID_t = uint256;
    using NodeID_t  = NodeID;
};

}

#endif
