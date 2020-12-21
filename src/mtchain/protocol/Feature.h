//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_FEATURE_H_INCLUDED
#define MTCHAIN_PROTOCOL_FEATURE_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <string>

namespace mtchain {

/** Convert feature description to feature id. */
/** @{ */
uint256
feature (std::string const& name);

uint256
feature (const char* name);
/** @} */

extern uint256 const featureMultiSign;
extern uint256 const featureTickets;
extern uint256 const featureTrustSetAuth;
extern uint256 const featureFeeEscalation;
extern uint256 const featureOwnerPaysFee;
extern uint256 const featureCompareFlowV1V2;
extern uint256 const featureSHAMapV2;
extern uint256 const featurePayChan;
extern uint256 const featureFlow;
extern uint256 const featureCryptoConditions;
extern uint256 const featureTickSize;
extern uint256 const fix1368;
extern uint256 const featureEscrow;
extern uint256 const featureCryptoConditionsSuite;

} //

#endif
