//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_APPLY_H_INCLUDED
#define MTCHAIN_TX_APPLY_H_INCLUDED

#include <mtchain/core/Config.h>
#include <mtchain/ledger/View.h>
#include <mtchain/protocol/STTx.h>
#include <mtchain/protocol/TER.h>
#include <mtchain/beast/utility/Journal.h>
#include <memory>
#include <utility>

namespace mtchain {

class Application;
class HashRouter;

enum class Validity
{
    SigBad,         // Signature is bad. Didn't do local checks.
    SigGoodOnly,    // Signature is good, but local checks fail.
    Valid           // Signature and local checks are good / passed.
};

/** Checks transaction signature and local checks. Returns
    a Validity enum representing how valid the STTx is
    and, if not Valid, a reason string.
    Results are cached internally, so tests will not be
    repeated over repeated calls, unless cache expires.

    @return std::pair, where `.first` is the status, and
            `.second` is the reason if appropriate.
*/
std::pair<Validity, std::string>
checkValidity(HashRouter& router,
    STTx const& tx, Rules const& rules,
        Config const& config);


/** Sets the validity of a given transaction in the cache.
    Use with extreme care.

    @note Can only raise the validity to a more valid state,
          and can not override anything cached bad.
*/
void
forceValidity(HashRouter& router, uint256 const& txid,
    Validity validity);

/** Apply a transaction to a ReadView.

    Throws:
        Does not throw.

        For open ledgers, the Transactor will catch and
        return tefEXCEPTION. For closed ledgers, the
        Transactor will attempt to only charge a fee,
        and return tecFAILED_PROCESSING.

        If the Transactor gets an exception while trying
        to charge the fee, it will be caught here and
        turned into tefEXCEPTION.

        This try/catch handler is the last resort, any
        uncaught exceptions will be turned into
        tefEXCEPTION.

        For network health, a Transactor makes its
        best effort to at least charge a fee if the
        ledger is closed.

    @return A pair with the TER and a bool indicating
            whether or not the transaction was applied.
*/
std::pair<TER, bool>
apply (Application& app, OpenView& view,
    STTx const& tx, ApplyFlags flags,
        beast::Journal journal);


/** Class for return value from applyTransaction */
enum class ApplyResult
{
    Success,    // Applied to this ledger
    Fail,       // Should not be retried in this ledger
    Retry       // Should be retried in this ledger
};

/** Transaction application helper

    Provides more detailed logging and decodes the
    correct behavior based on the TER type
*/
ApplyResult
applyTransaction(Application& app, OpenView& view,
    STTx const& tx, bool retryAssured, ApplyFlags flags,
    beast::Journal journal);

void transferAmount(Application& app, AccountID const& dstAccount, STAmount const& amount,
    OpenView &view, STObject *memo = nullptr);
} //

#endif
