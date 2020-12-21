//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_APPLYSTEPS_H_INCLUDED
#define MTCHAIN_TX_APPLYSTEPS_H_INCLUDED

#include <mtchain/ledger/ApplyViewImpl.h>
#include <mtchain/beast/utility/Journal.h>

namespace mtchain {

class Application;
class STTx;

struct PreflightResult
{
public:
    // from the context
    STTx const& tx;
    Rules const rules;
    ApplyFlags const flags;
    beast::Journal const j;

    // result
    TER const ter;

    template<class Context>
    PreflightResult(Context const& ctx_,
        TER ter_)
        : tx(ctx_.tx)
        , rules(ctx_.rules)
        , flags(ctx_.flags)
        , j(ctx_.j)
        , ter(ter_)
    {
    }

    PreflightResult& operator=(PreflightResult const&) = delete;
};

struct PreclaimResult
{
public:
    // from the context
    ReadView const& view;
    STTx const& tx;
    ApplyFlags const flags;
    beast::Journal const j;

    // result
    TER const ter;
    std::uint64_t const baseFee;
    bool const likelyToClaimFee;

    template<class Context>
    PreclaimResult(Context const& ctx_,
        TER ter_, std::uint64_t const& baseFee_)
        : view(ctx_.view)
        , tx(ctx_.tx)
        , flags(ctx_.flags)
        , j(ctx_.j)
        , ter(ter_)
        , baseFee(baseFee_)
        , likelyToClaimFee(ter == tesSUCCESS
            || isTecClaim(ter))
    {
    }

    template<class Context>
    PreclaimResult(Context const& ctx_,
        std::pair<TER, std::uint64_t> const& result)
        : PreclaimResult(ctx_, result.first, result.second)
    {
    }

    PreclaimResult& operator=(PreclaimResult const&) = delete;
};

struct TxConsequences
{
    enum Category
    {
        // Moves currency around, creates offers, etc.
        normal = 0,
        // Affects the ability of subsequent transactions
        // to claim a fee. Eg. SetRegularKey
        blocker
    };

    Category const category;
    MAmount const fee;
    // Does NOT include the fee.
    MAmount const potentialSpend;

    TxConsequences(Category const category_,
        MAmount const fee_, MAmount const spend_)
        : category(category_)
        , fee(fee_)
        , potentialSpend(spend_)
    {
    }

    TxConsequences(TxConsequences const&) = default;
    TxConsequences& operator=(TxConsequences const&) = delete;
    TxConsequences(TxConsequences&&) = default;
    TxConsequences& operator=(TxConsequences&&) = delete;

};

/** Gate a transaction based on static information.

    The transaction is checked against all possible
    validity constraints that do not require a ledger.

    @return A PreflightResult object constaining, among
    other things, the TER code.
*/
PreflightResult
preflight(Application& app, Rules const& rules,
    STTx const& tx, ApplyFlags flags,
        beast::Journal j);

/** Gate a transaction based on static ledger information.

    The transaction is checked against all possible
    validity constraints that DO require a ledger.

    If preclaim succeeds, then the transaction is very
    likely to claim a fee. This will determine if the
    transaction is safe to relay without being applied
    to the open ledger.

    "Succeeds" in this case is defined as returning a
    `tes` or `tec`, since both lead to claiming a fee.

    @return A PreclaimResult object containing, among
    other things the TER code and the base fee value for
    this transaction.
*/
PreclaimResult
preclaim(PreflightResult const& preflightResult,
    Application& app, OpenView const& view);

/** Compute only the expected base fee for a transaction.

    Base fees are transaction specific, so any calculation
    needing them must get the base fee for each transaction.

    No validation is done or implied by this function.

    Caller is responsible for handling any exceptions.
    Since none should be thrown, that will usually
    mean terminating.

    @return The base fee.
*/
std::uint64_t
calculateBaseFee(Application& app, ReadView const& view,
    STTx const& tx, beast::Journal j);

/** Determine the M balance consequences if a transaction
    consumes the maximum M allowed.
*/
TxConsequences
calculateConsequences(PreflightResult const& preflightResult);

/** Apply a prechecked transaction to an OpenView.

    See also: apply()

    Precondition: The transaction has been checked
    and validated using the above functions.

    @return A pair with the TER and a bool indicating
    whether or not the transaction was applied.
*/
std::pair<TER, bool>
doApply(PreclaimResult const& preclaimResult,
    Application& app, OpenView& view);

void
afterApply(ReadView const& view, STTx const& tx, TER terResult,
           Application& app, beast::Journal& j);

}

#endif
