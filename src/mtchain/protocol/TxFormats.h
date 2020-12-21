//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_TXFORMATS_H_INCLUDED
#define MTCHAIN_PROTOCOL_TXFORMATS_H_INCLUDED

#include <mtchain/protocol/KnownFormats.h>

namespace mtchain {

/** Transaction type identifiers.

    These are part of the binary message format.

    @ingroup protocol
*/
enum TxType
{
    ttINVALID           = -1,

    ttPAYMENT           = 0,
    ttESCROW_CREATE     = 1,
    ttESCROW_FINISH     = 2,
    ttACCOUNT_SET       = 3,
    ttESCROW_CANCEL     = 4,
    ttREGULAR_KEY_SET   = 5,
    ttNICKNAME_SET      = 6, // open
    ttOFFER_CREATE      = 7,
    ttOFFER_CANCEL      = 8,
    no_longer_used      = 9,
    ttTICKET_CREATE     = 10,
    ttTICKET_CANCEL     = 11,
    ttSIGNER_LIST_SET   = 12,
    ttPAYCHAN_CREATE    = 13,
    ttPAYCHAN_FUND      = 14,
    ttPAYCHAN_CLAIM     = 15,
    ttMULTIPAYMENT      = 16,

    ttTRUST_SET         = 20,

    ttASSET_CREATE      = 32,
    ttTOKEN_CREATE      = 33,
    ttTOKEN_DESTROY     = 34,
    ttTOKEN_REVOKE      = 35,
    ttTOKEN_TRANSFER    = 36,
    ttTOKEN_APPROVE     = 37,
    ttTOKEN_APPROVE_CANCEL = 38,
    ttASSET_APPROVE     = 39,
    ttASSET_APPROVE_CANCEL = 40,
    ttASSET_DESTROY     = 41,
    ttASSET_SET         = 42,
    ttTOKEN_SET         = 43,

    ttAMENDMENT         = 100,
    ttFEE               = 101,
};

/** Manages the list of known transaction formats.
*/
class TxFormats : public KnownFormats <TxType>
{
private:
    void addCommonFields (Item& item);

public:
    /** Create the object.
        This will load the object will all the known transaction formats.
    */
    TxFormats ();

    static TxFormats const& getInstance ();
};

} //

#endif
