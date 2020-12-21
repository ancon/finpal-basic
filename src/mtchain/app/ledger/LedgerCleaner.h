//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_LEDGERCLEANER_H_INCLUDED
#define MTCHAIN_APP_LEDGER_LEDGERCLEANER_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/json/json_value.h>
#include <mtchain/core/Stoppable.h>
#include <mtchain/beast/utility/PropertyStream.h>
#include <mtchain/beast/utility/Journal.h>
#include <memory>

namespace mtchain {
namespace detail {

/** Check the ledger/transaction databases to make sure they have continuity */
class LedgerCleaner
    : public Stoppable
    , public beast::PropertyStream::Source
{
protected:
    explicit LedgerCleaner (Stoppable& parent);

public:
    /** Destroy the object. */
    virtual ~LedgerCleaner () = 0;

    /** Start a long running task to clean the ledger.
        The ledger is cleaned asynchronously, on an implementation defined
        thread. This function call does not block. The long running task
        will be stopped if the Stoppable stops.

        Thread safety:
            Safe to call from any thread at any time.

        @param parameters A Json object with configurable parameters.
    */
    virtual void doClean (Json::Value const& parameters) = 0;
};

std::unique_ptr<LedgerCleaner>
make_LedgerCleaner (Application& app,
    Stoppable& parent, beast::Journal journal);

} // detail
} //

#endif
