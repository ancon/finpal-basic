//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/protocol/Feature.h>
#include <cstring>

namespace mtchain {

static
uint256
feature (char const* s, std::size_t n)
{
    sha512_half_hasher h;
    h(s, n);
    return static_cast<uint256>(h);
}

uint256
feature (std::string const& name)
{
    return feature(name.c_str(), name.size());
}

uint256
feature (const char* name)
{
    return feature(name, std::strlen(name));
}

uint256 const featureMultiSign = feature("MultiSign");
uint256 const featureTickets = feature("Tickets");
uint256 const featureTrustSetAuth = feature("TrustSetAuth");
uint256 const featureFeeEscalation = feature("FeeEscalation");
uint256 const featureOwnerPaysFee = feature("OwnerPaysFee");
uint256 const featureCompareFlowV1V2 = feature("CompareFlowV1V2");
uint256 const featureSHAMapV2 = feature("SHAMapV2");
uint256 const featurePayChan = feature("PayChan");
uint256 const featureFlow = feature("Flow");
uint256 const featureCryptoConditions = feature("CryptoConditions");
uint256 const featureTickSize = feature("TickSize");
uint256 const fix1368 = feature("fix1368");
uint256 const featureEscrow = feature("Escrow");
uint256 const featureCryptoConditionsSuite = feature("CryptoConditionsSuite");

} //
