//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_TXFLAGS_H_INCLUDED
#define MTCHAIN_PROTOCOL_TXFLAGS_H_INCLUDED

#include <cstdint>

namespace mtchain {

//
// Transaction flags.
//

/** Transaction flags.

    These flags modify the behavior of an operation.

    @note Changing these will create a hard fork
    @ingroup protocol
*/
class TxFlag
{
public:
    static std::uint32_t const requireDestTag = 0x00010000;
};
// VFALCO TODO Move all flags into this container after some study.

// Universal Transaction flags:
const std::uint32_t tfFullyCanonicalSig    = 0x80000000;
const std::uint32_t tfUniversal            = tfFullyCanonicalSig;
const std::uint32_t tfUniversalMask        = ~ tfUniversal;

// AccountSet flags:
// VFALCO TODO Javadoc comment every one of these constants
//const std::uint32_t TxFlag::requireDestTag       = 0x00010000;
const std::uint32_t tfOptionalDestTag      = 0x00020000;
const std::uint32_t tfRequireAuth          = 0x00040000;
const std::uint32_t tfOptionalAuth         = 0x00080000;
const std::uint32_t tfDisallowM            = 0x00100000;
const std::uint32_t tfAllowM               = 0x00200000;
const std::uint32_t tfAccountSetMask       = ~ (tfUniversal | TxFlag::requireDestTag | tfOptionalDestTag | tfRequireAuth | tfOptionalAuth | tfDisallowM | tfAllowM);

// AccountSet SetFlag/ClearFlag values
const std::uint32_t asfRequireDest         = 1;
const std::uint32_t asfRequireAuth         = 2;
const std::uint32_t asfDisallowM           = 3;
const std::uint32_t asfDisableMaster       = 4;
const std::uint32_t asfAccountTxnID        = 5;
const std::uint32_t asfNoFreeze            = 6;
const std::uint32_t asfGlobalFreeze        = 7;
const std::uint32_t asfDefaultMtchain      = 8;
const std::uint32_t asfIOUPrecision        = 16;

// OfferCreate flags:
const std::uint32_t tfPassive              = 0x00010000;
const std::uint32_t tfImmediateOrCancel    = 0x00020000;
const std::uint32_t tfFillOrKill           = 0x00040000;
const std::uint32_t tfSell                 = 0x00080000;
const std::uint32_t tfOfferCreateMask      = ~ (tfUniversal | tfPassive | tfImmediateOrCancel | tfFillOrKill | tfSell);

// Payment flags:
const std::uint32_t tfNoMTChainDirect      = 0x00010000;
const std::uint32_t tfPartialPayment       = 0x00020000;
const std::uint32_t tfLimitQuality         = 0x00040000;
const std::uint32_t tfPaymentMask          = ~ (tfUniversal | tfPartialPayment | tfLimitQuality | tfNoMTChainDirect);

// TrustSet flags:
const std::uint32_t tfSetfAuth             = 0x00010000;
const std::uint32_t tfSetNoMtchain         = 0x00020000;
const std::uint32_t tfClearNoMtchain       = 0x00040000;
const std::uint32_t tfSetFreeze            = 0x00100000;
const std::uint32_t tfClearFreeze          = 0x00200000;
const std::uint32_t tfTrustSetMask         = ~ (tfUniversal | tfSetfAuth | tfSetNoMtchain | tfClearNoMtchain | tfSetFreeze | tfClearFreeze);

// EnableAmendment flags:
const std::uint32_t tfGotMajority          = 0x00010000;
const std::uint32_t tfLostMajority         = 0x00020000;

// PaymentChannel flags:
const std::uint32_t tfRenew                = 0x00010000;
const std::uint32_t tfClose                = 0x00020000;

// AssetCapabilities
const std::uint32_t tfTransferToken        = 0x00000001;
const std::uint32_t tfDestroyToken         = 0x00000002;
const std::uint32_t tfUpdateToken          = 0x00000004;
const std::uint32_t tfToggleTransfer       = 0x00000008;
const std::uint32_t tfRevokeToken          = 0x00000010;
const std::uint32_t tfAssetCapMask         = ~ (tfUniversal | tfTransferToken | tfDestroyToken | tfUpdateToken | tfToggleTransfer | tfRevokeToken);

// AssetSet Flags
const std::uint32_t asfAllowTransferToken  = 1;

} //

#endif
