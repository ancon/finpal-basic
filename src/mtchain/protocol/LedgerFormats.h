//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_LEDGERFORMATS_H_INCLUDED
#define MTCHAIN_PROTOCOL_LEDGERFORMATS_H_INCLUDED

#include <mtchain/protocol/KnownFormats.h>

namespace mtchain {

/** Ledger entry types.

    These are stored in serialized data.

    @note Changing these values results in a hard fork.

    @ingroup protocol
*/
// Used as the type of a transaction or the type of a ledger entry.
enum LedgerEntryType
{
    /** Special type, anything
        This is used when the type in the Keylet is unknown,
        such as when building metadata.
    */
    ltANY = -3,

    /** Special type, anything not a directory
        This is used when the type in the Keylet is unknown,
        such as when iterating
    */
    ltCHILD             = -2,

    ltINVALID           = -1,

    //---------------------------------------------------------------------------

    ltACCOUNT_ROOT      = 'a',

    /** Directory node.

        A directory is a vector 256-bit values. Usually they represent
        hashes of other objects in the ledger.

        Used in an append-only fashion.

        (There's a little more information than this, see the template)
    */
    ltDIR_NODE          = 'd',

    ltMTCHAIN_STATE     = 'r',

    ltTICKET            = 'T',

    ltSIGNER_LIST       = 'S',

    ltOFFER             = 'o',

    ltLEDGER_HASHES     = 'h',

    ltAMENDMENTS        = 'f',

    ltFEE_SETTINGS      = 's',

    ltESCROW            = 'u',

    // Simple unidirection m channel
    ltPAYCHAN           = 'x',

    // No longer used or supported. Left here to prevent accidental
    // reassignment of the ledger type.
    ltNICKNAME          = 'n',

    ltNotUsed01         = 'c',
    ltNFASSET           = 'N',
    ltNFTOKEN           = 't',
    ltNFTAUTH           = 'U',
    ltNFT_ACCOUNT       = 'A',
    ltNFT_INDEX         = 'I',
    ltNFT_OWNER_INDEX   = 'i',
    ltNFASSET_INDEX     = 'P',
};

/**
    @ingroup protocol
*/
// Used as a prefix for computing ledger indexes (keys).
enum LedgerNameSpace
{
    spaceAccount        = 'a',
    spaceDirNode        = 'd',
    spaceGenerator      = 'g',
    spaceMTChain        = 'r',
    spaceOffer          = 'o',  // Entry for an offer.
    spaceOwnerDir       = 'O',  // Directory of things owned by an account.
    spaceBookDir        = 'B',  // Directory of order books.
    spaceContract       = 'c',
    spaceSkipList       = 's',
    spaceEscrow         = 'u',
    spaceAmendment      = 'f',
    spaceFee            = 'e',
    spaceTicket         = 'T',
    spaceSignerList     = 'S',
    spaceMUChannel    = 'x',

    // No longer used or supported. Left here to reserve the space and
    // avoid accidental reuse of the space.
    spaceNickname       = 'n',
    spaceNFAsset        = 'N',
    spaceNFToken        = 't',
    spaceNFTAuth        = 'U',
    spaceNFTAccount     = 'A',
    spaceNFTIndex       = 'I',
    spaceNFTOwnerIndex  = 'i',
    spaceNFAssetIndex   = 'P',
};

/**
    @ingroup protocol
*/
enum LedgerSpecificFlags
{
    // ltACCOUNT_ROOT
    lsfPasswordSpent    = 0x00010000,   // True, if password set fee is spent.
    lsfRequireDestTag   = 0x00020000,   // True, to require a DestinationTag for payments.
    lsfRequireAuth      = 0x00040000,   // True, to require a authorization to hold IOUs.
    lsfDisallowM        = 0x00080000,   // True, to disallow sending M.
    lsfDisableMaster    = 0x00100000,   // True, force regular key
    lsfNoFreeze         = 0x00200000,   // True, cannot freeze mtchain states
    lsfGlobalFreeze     = 0x00400000,   // True, all assets frozen
    lsfDefaultMtchain   = 0x00800000,   // True, trust lines allow rippling by default

    // ltOFFER
    lsfPassive          = 0x00010000,
    lsfSell             = 0x00020000,   // True, offer was placed as a sell.

    // ltMTCHAIN_STATE
    lsfLowReserve       = 0x00010000,   // True, if entry counts toward reserve.
    lsfHighReserve      = 0x00020000,
    lsfLowAuth          = 0x00040000,
    lsfHighAuth         = 0x00080000,
    lsfLowNoMtchain     = 0x00100000,
    lsfHighNoMtchain    = 0x00200000,
    lsfLowFreeze        = 0x00400000,   // True, low side has set freeze flag
    lsfHighFreeze       = 0x00800000,   // True, high side has set freeze flag
};

//------------------------------------------------------------------------------

/** Holds the list of known ledger entry formats.
*/
class LedgerFormats : public KnownFormats <LedgerEntryType>
{
private:
    LedgerFormats ();

public:
    static LedgerFormats const& getInstance ();

private:
    void addCommonFields (Item& item);
};

} //

#endif
