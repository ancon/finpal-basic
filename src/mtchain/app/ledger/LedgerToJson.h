//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_LEDGERTOJSON_H_INCLUDED
#define MTCHAIN_APP_LEDGER_LEDGERTOJSON_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/STTx.h>
#include <mtchain/json/Object.h>

namespace mtchain {

struct LedgerFill
{
    LedgerFill (ReadView const& l, int o = 0)
        : ledger (l)
        , options (o)
    {
    }

    enum Options {
        dumpTm = 1, dumpState = 2, expand = 4, full = 8, binary = 16,
        ownerFunds = 32};

    ReadView const& ledger;
    int options;
};

/** Given a Ledger and options, fill a Json::Object or Json::Value with a
    description of the ledger.
 */

void addJson(Json::Value&, LedgerFill const&);
void addJson(Json::Object&, LedgerFill const&);

/** Return a new Json::Value representing the ledger with given options.*/
Json::Value getJson (LedgerFill const&);

/** Serialize an object to a blob. */
template <class Object>
Blob serializeBlob(Object const& o)
{
    Serializer s;
    o.add(s);
    return s.peekData();
}

/** Serialize an object to a hex string. */
inline
std::string serializeHex(STObject const& o)
{
    return strHex(serializeBlob(o));
}
} //

#endif
