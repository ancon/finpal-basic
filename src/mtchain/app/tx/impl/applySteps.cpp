//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/applySteps.h>
#include <mtchain/app/tx/impl/ApplyContext.h>
#include <mtchain/app/tx/impl/CancelOffer.h>
#include <mtchain/app/tx/impl/CancelTicket.h>
#include <mtchain/app/tx/impl/Change.h>
#include <mtchain/app/tx/impl/CreateOffer.h>
#include <mtchain/app/tx/impl/CreateTicket.h>
#include <mtchain/app/tx/impl/Escrow.h>
#include <mtchain/app/tx/impl/Payment.h>
#include <mtchain/app/tx/impl/SetAccount.h>
#include <mtchain/app/tx/impl/SetRegularKey.h>
#include <mtchain/app/tx/impl/SetSignerList.h>
#include <mtchain/app/tx/impl/SetTrust.h>
#include <mtchain/app/tx/impl/PayChan.h>
#include <mtchain/app/tx/impl/NFAsset.h>
#include <mtchain/app/tx/impl/NFToken.h>

namespace mtchain {

static
TER
invoke_preflight (PreflightContext const& ctx)
{
    switch(ctx.tx.getTxnType())
    {
    case ttACCOUNT_SET:     return SetAccount       ::preflight(ctx);
    case ttOFFER_CANCEL:    return CancelOffer      ::preflight(ctx);
    case ttOFFER_CREATE:    return CreateOffer      ::preflight(ctx);
    case ttPAYMENT:
    case ttMULTIPAYMENT:    return Payment          ::preflight(ctx);
    case ttESCROW_CREATE:   return EscrowCreate     ::preflight(ctx);
    case ttESCROW_FINISH:   return EscrowFinish     ::preflight(ctx);
    case ttESCROW_CANCEL:   return EscrowCancel     ::preflight(ctx);
    case ttREGULAR_KEY_SET: return SetRegularKey    ::preflight(ctx);
    case ttSIGNER_LIST_SET: return SetSignerList    ::preflight(ctx);
    case ttTICKET_CANCEL:   return CancelTicket     ::preflight(ctx);
    case ttTICKET_CREATE:   return CreateTicket     ::preflight(ctx);
    case ttTRUST_SET:       return SetTrust         ::preflight(ctx);
    case ttAMENDMENT:
    case ttFEE:             return Change           ::preflight(ctx);
    case ttPAYCHAN_CREATE:  return PayChanCreate    ::preflight(ctx);
    case ttPAYCHAN_FUND:    return PayChanFund      ::preflight(ctx);
    case ttPAYCHAN_CLAIM:   return PayChanClaim     ::preflight(ctx);
    case ttASSET_CREATE:    return CreateAsset      ::preflight(ctx);
    case ttTOKEN_CREATE:    return CreateToken      ::preflight(ctx);
    case ttTOKEN_DESTROY:   return DestroyToken     ::preflight(ctx);
    case ttTOKEN_REVOKE:    return RevokeToken      ::preflight(ctx);
    case ttTOKEN_TRANSFER:  return TransferToken    ::preflight(ctx);
    case ttTOKEN_APPROVE:   return ApproveToken     ::preflight(ctx);
    case ttTOKEN_APPROVE_CANCEL: return CancelApproveToken::preflight(ctx);
    case ttASSET_APPROVE:   return ApproveAsset     ::preflight(ctx);
    case ttASSET_APPROVE_CANCEL: return CancelApproveAsset::preflight(ctx);
    case ttASSET_DESTROY:   return DestroyAsset     ::preflight(ctx);
    case ttASSET_SET:       return SetAsset         ::preflight(ctx);
    case ttTOKEN_SET:       return SetToken         ::preflight(ctx);
    default:
        assert(false);
        return temUNKNOWN;
    }
}

/* invoke_preclaim<T> uses name hiding to accomplish
    compile-time polymorphism of (presumably) static
    class functions for Transactor and derived classes.
*/
template<class T>
static
std::pair<TER, std::uint64_t>
invoke_preclaim(PreclaimContext const& ctx)
{
    // If the transactor requires a valid account and the transaction doesn't
    // list one, preflight will have already a flagged a failure.
    auto const id = ctx.tx.getAccountID(sfAccount);
    auto const baseFee = T::calculateBaseFee(ctx);
    TER result;
    if (id != zero)
    {
        result = T::checkSeq(ctx);

        if (result != tesSUCCESS)
            return { result, baseFee };

        result = T::checkFee(ctx, baseFee);

        if (result != tesSUCCESS)
            return { result, baseFee };

        result = T::checkSign(ctx);

        if (result != tesSUCCESS)
            return { result, baseFee };

        result = T::preclaim(ctx);

        if (result != tesSUCCESS)
            return{ result, baseFee };
    }
    else
    {
        result = tesSUCCESS;
    }

    return { tesSUCCESS, baseFee };
}

static
std::pair<TER, std::uint64_t>
invoke_preclaim (PreclaimContext const& ctx)
{
    switch(ctx.tx.getTxnType())
    {
    case ttACCOUNT_SET:     return invoke_preclaim<SetAccount>(ctx);
    case ttOFFER_CANCEL:    return invoke_preclaim<CancelOffer>(ctx);
    case ttOFFER_CREATE:    return invoke_preclaim<CreateOffer>(ctx);
    case ttPAYMENT:
    case ttMULTIPAYMENT:    return invoke_preclaim<Payment>(ctx);
    case ttESCROW_CREATE:   return invoke_preclaim<EscrowCreate>(ctx);
    case ttESCROW_FINISH:   return invoke_preclaim<EscrowFinish>(ctx);
    case ttESCROW_CANCEL:   return invoke_preclaim<EscrowCancel>(ctx);
    case ttREGULAR_KEY_SET: return invoke_preclaim<SetRegularKey>(ctx);
    case ttSIGNER_LIST_SET: return invoke_preclaim<SetSignerList>(ctx);
    case ttTICKET_CANCEL:   return invoke_preclaim<CancelTicket>(ctx);
    case ttTICKET_CREATE:   return invoke_preclaim<CreateTicket>(ctx);
    case ttTRUST_SET:       return invoke_preclaim<SetTrust>(ctx);
    case ttAMENDMENT:
    case ttFEE:             return invoke_preclaim<Change>(ctx);
    case ttPAYCHAN_CREATE:  return invoke_preclaim<PayChanCreate>(ctx);
    case ttPAYCHAN_FUND:    return invoke_preclaim<PayChanFund>(ctx);
    case ttPAYCHAN_CLAIM:   return invoke_preclaim<PayChanClaim>(ctx);
    case ttASSET_CREATE:    return invoke_preclaim<CreateAsset>(ctx);
    case ttTOKEN_CREATE:    return invoke_preclaim<CreateToken>(ctx);
    case ttTOKEN_DESTROY:   return invoke_preclaim<DestroyToken>(ctx);
    case ttTOKEN_REVOKE:    return invoke_preclaim<RevokeToken>(ctx);
    case ttTOKEN_TRANSFER:  return invoke_preclaim<TransferToken>(ctx);
    case ttTOKEN_APPROVE:   return invoke_preclaim<ApproveToken>(ctx);
    case ttTOKEN_APPROVE_CANCEL: return invoke_preclaim<CancelApproveToken>(ctx);
    case ttASSET_APPROVE:   return invoke_preclaim<ApproveAsset>(ctx);
    case ttASSET_APPROVE_CANCEL: return invoke_preclaim<CancelApproveAsset>(ctx);
    case ttASSET_DESTROY:   return invoke_preclaim<DestroyAsset>(ctx);
    case ttASSET_SET:       return invoke_preclaim<SetAsset>(ctx);
    case ttTOKEN_SET:       return invoke_preclaim<SetToken>(ctx);
    default:
        assert(false);
        return { temUNKNOWN, 0 };
    }
}

static
std::uint64_t
invoke_calculateBaseFee(PreclaimContext const& ctx)
{
    switch (ctx.tx.getTxnType())
    {
    case ttACCOUNT_SET:     return SetAccount::calculateBaseFee(ctx);
    case ttOFFER_CANCEL:    return CancelOffer::calculateBaseFee(ctx);
    case ttOFFER_CREATE:    return CreateOffer::calculateBaseFee(ctx);
    case ttPAYMENT:
    case ttMULTIPAYMENT:    return Payment::calculateBaseFee(ctx);
    case ttESCROW_CREATE:   return EscrowCreate::calculateBaseFee(ctx);
    case ttESCROW_FINISH:   return EscrowFinish::calculateBaseFee(ctx);
    case ttESCROW_CANCEL:   return EscrowCancel::calculateBaseFee(ctx);
    case ttREGULAR_KEY_SET: return SetRegularKey::calculateBaseFee(ctx);
    case ttSIGNER_LIST_SET: return SetSignerList::calculateBaseFee(ctx);
    case ttTICKET_CANCEL:   return CancelTicket::calculateBaseFee(ctx);
    case ttTICKET_CREATE:   return CreateTicket::calculateBaseFee(ctx);
    case ttTRUST_SET:       return SetTrust::calculateBaseFee(ctx);
    case ttAMENDMENT:
    case ttFEE:             return Change::calculateBaseFee(ctx);
    case ttPAYCHAN_CREATE:  return PayChanCreate::calculateBaseFee(ctx);
    case ttPAYCHAN_FUND:    return PayChanFund::calculateBaseFee(ctx);
    case ttPAYCHAN_CLAIM:   return PayChanClaim::calculateBaseFee(ctx);
    case ttASSET_CREATE:    return CreateAsset::calculateBaseFee(ctx);
    case ttTOKEN_CREATE:    return CreateToken::calculateBaseFee(ctx);
    case ttTOKEN_DESTROY:   return DestroyToken::calculateBaseFee(ctx);
    case ttTOKEN_REVOKE:    return RevokeToken::calculateBaseFee(ctx);
    case ttTOKEN_TRANSFER:  return TransferToken::calculateBaseFee(ctx);
    case ttTOKEN_APPROVE:   return ApproveToken::calculateBaseFee(ctx);
    case ttTOKEN_APPROVE_CANCEL: return CancelApproveToken::calculateBaseFee(ctx);
    case ttASSET_APPROVE:   return ApproveAsset::calculateBaseFee(ctx);
    case ttASSET_APPROVE_CANCEL: return CancelApproveAsset::calculateBaseFee(ctx);
    case ttASSET_DESTROY:   return DestroyAsset::calculateBaseFee(ctx);
    case ttASSET_SET:       return SetAsset::calculateBaseFee(ctx);
    case ttTOKEN_SET:       return SetToken::calculateBaseFee(ctx);
    default:
        assert(false);
        return 0;
    }
}

template<class T>
static
TxConsequences
invoke_calculateConsequences(STTx const& tx)
{
    auto const category = T::affectsSubsequentTransactionAuth(tx) ?
        TxConsequences::blocker : TxConsequences::normal;
    auto const feePaid = T::calculateFeePaid(tx);
    auto const maxSpend = T::calculateMaxSpend(tx);

    return{ category, feePaid, maxSpend };
}

static
TxConsequences
invoke_calculateConsequences(STTx const& tx)
{
    switch (tx.getTxnType())
    {
    case ttACCOUNT_SET:     return invoke_calculateConsequences<SetAccount>(tx);
    case ttOFFER_CANCEL:    return invoke_calculateConsequences<CancelOffer>(tx);
    case ttOFFER_CREATE:    return invoke_calculateConsequences<CreateOffer>(tx);
    case ttPAYMENT:
    case ttMULTIPAYMENT:    return invoke_calculateConsequences<Payment>(tx);
    case ttESCROW_CREATE:   return invoke_calculateConsequences<EscrowCreate>(tx);
    case ttESCROW_FINISH:   return invoke_calculateConsequences<EscrowFinish>(tx);
    case ttESCROW_CANCEL:   return invoke_calculateConsequences<EscrowCancel>(tx);
    case ttREGULAR_KEY_SET: return invoke_calculateConsequences<SetRegularKey>(tx);
    case ttSIGNER_LIST_SET: return invoke_calculateConsequences<SetSignerList>(tx);
    case ttTICKET_CANCEL:   return invoke_calculateConsequences<CancelTicket>(tx);
    case ttTICKET_CREATE:   return invoke_calculateConsequences<CreateTicket>(tx);
    case ttTRUST_SET:       return invoke_calculateConsequences<SetTrust>(tx);
    case ttPAYCHAN_CREATE:  return invoke_calculateConsequences<PayChanCreate>(tx);
    case ttPAYCHAN_FUND:    return invoke_calculateConsequences<PayChanFund>(tx);
    case ttPAYCHAN_CLAIM:   return invoke_calculateConsequences<PayChanClaim>(tx);
    case ttASSET_CREATE:    return invoke_calculateConsequences<CreateAsset>(tx);
    case ttTOKEN_CREATE:    return invoke_calculateConsequences<CreateToken>(tx);
    case ttTOKEN_DESTROY:   return invoke_calculateConsequences<DestroyToken>(tx);
    case ttTOKEN_REVOKE:    return invoke_calculateConsequences<RevokeToken>(tx);
    case ttTOKEN_TRANSFER:  return invoke_calculateConsequences<TransferToken>(tx);
    case ttTOKEN_APPROVE:   return invoke_calculateConsequences<ApproveToken>(tx);
    case ttTOKEN_APPROVE_CANCEL: return invoke_calculateConsequences<CancelApproveToken>(tx);
    case ttASSET_APPROVE:   return invoke_calculateConsequences<ApproveAsset>(tx);
    case ttASSET_APPROVE_CANCEL: invoke_calculateConsequences<CancelApproveAsset>(tx);
    case ttASSET_DESTROY:   return invoke_calculateConsequences<DestroyAsset>(tx);
    case ttASSET_SET:       return invoke_calculateConsequences<SetAsset>(tx);
    case ttTOKEN_SET:       return invoke_calculateConsequences<SetToken>(tx);
    case ttAMENDMENT:
    case ttFEE:
        // fall through to default
    default:
        assert(false);
        return { TxConsequences::blocker, Transactor::calculateFeePaid(tx),
            beast::zero };
    }
}

static
std::pair<TER, bool>
invoke_apply (ApplyContext& ctx)
{
    switch(ctx.tx.getTxnType())
    {
    case ttACCOUNT_SET:     { SetAccount    p(ctx); return p(); }
    case ttOFFER_CANCEL:    { CancelOffer   p(ctx); return p(); }
    case ttOFFER_CREATE:    { CreateOffer   p(ctx); return p(); }
    case ttPAYMENT:
    case ttMULTIPAYMENT:    { Payment       p(ctx); return p(); }
    case ttESCROW_CREATE:   { EscrowCreate  p(ctx); return p(); }
    case ttESCROW_FINISH:   { EscrowFinish  p(ctx); return p(); }
    case ttESCROW_CANCEL:   { EscrowCancel  p(ctx); return p(); }
    case ttREGULAR_KEY_SET: { SetRegularKey p(ctx); return p(); }
    case ttSIGNER_LIST_SET: { SetSignerList p(ctx); return p(); }
    case ttTICKET_CANCEL:   { CancelTicket  p(ctx); return p(); }
    case ttTICKET_CREATE:   { CreateTicket  p(ctx); return p(); }
    case ttTRUST_SET:       { SetTrust      p(ctx); return p(); }
    case ttAMENDMENT:
    case ttFEE:             { Change        p(ctx); return p(); }
    case ttPAYCHAN_CREATE:  { PayChanCreate p(ctx); return p(); }
    case ttPAYCHAN_FUND:    { PayChanFund   p(ctx); return p(); }
    case ttPAYCHAN_CLAIM:   { PayChanClaim  p(ctx); return p(); }
    case ttASSET_CREATE:    { CreateAsset   p(ctx); return p(); }
    case ttTOKEN_CREATE:    { CreateToken   p(ctx); return p(); }
    case ttTOKEN_DESTROY:   { DestroyToken  p(ctx); return p(); }
    case ttTOKEN_REVOKE:    { RevokeToken   p(ctx); return p(); }
    case ttTOKEN_TRANSFER:  { TransferToken p(ctx); return p(); }
    case ttTOKEN_APPROVE:   { ApproveToken  p(ctx); return p(); }
    case ttTOKEN_APPROVE_CANCEL: { CancelApproveToken p(ctx); return p(); }
    case ttASSET_APPROVE:   { ApproveAsset  p(ctx); return p(); }
    case ttASSET_APPROVE_CANCEL: { CancelApproveAsset p(ctx); return p(); }
    case ttASSET_DESTROY:   { DestroyAsset  p(ctx); return p(); }
    case ttASSET_SET:       { SetAsset      p(ctx); return p(); }
    case ttTOKEN_SET:       { SetToken      p(ctx); return p(); }
    default:
        assert(false);
        return { temUNKNOWN, false };
    }
}

PreflightResult
preflight(Application& app, Rules const& rules,
    STTx const& tx, ApplyFlags flags,
        beast::Journal j)
{
    PreflightContext const pfctx(app, tx,
        rules, flags, j);
    try
    {
        return{ pfctx, invoke_preflight(pfctx) };
    }
    catch (std::exception const& e)
    {
        JLOG(j.fatal()) <<
            "apply: " << e.what();
        return{ pfctx, tefEXCEPTION };
    }
}

PreclaimResult
preclaim (PreflightResult const& preflightResult,
    Application& app, OpenView const& view)
{
    boost::optional<PreclaimContext const> ctx;
    if (preflightResult.rules != view.rules())
    {
        auto secondFlight = preflight(app, view.rules(),
            preflightResult.tx, preflightResult.flags,
                preflightResult.j);
        ctx.emplace(app, view, secondFlight.ter, secondFlight.tx,
            secondFlight.flags, secondFlight.j);
    }
    else
    {
        ctx.emplace(
            app, view, preflightResult.ter, preflightResult.tx,
                preflightResult.flags, preflightResult.j);
    }
    try
    {
        if (ctx->preflightResult != tesSUCCESS)
            return { *ctx, ctx->preflightResult, 0 };
        return{ *ctx, invoke_preclaim(*ctx) };
    }
    catch (std::exception const& e)
    {
        JLOG(ctx->j.fatal()) <<
            "apply: " << e.what();
        return{ *ctx, tefEXCEPTION, 0 };
    }
}

std::uint64_t
calculateBaseFee(Application& app, ReadView const& view,
    STTx const& tx, beast::Journal j)
{
    PreclaimContext const ctx(
        app, view, tesSUCCESS, tx,
            tapNONE, j);

    return invoke_calculateBaseFee(ctx);
}

TxConsequences
calculateConsequences(PreflightResult const& preflightResult)
{
    assert(preflightResult.ter == tesSUCCESS);
    if (preflightResult.ter != tesSUCCESS)
        return{ TxConsequences::blocker,
            Transactor::calculateFeePaid(preflightResult.tx),
                beast::zero };
    return invoke_calculateConsequences(preflightResult.tx);
}

std::pair<TER, bool>
doApply(PreclaimResult const& preclaimResult,
    Application& app, OpenView& view)
{
    if (preclaimResult.view.seq() != view.seq())
    {
        // Logic error from the caller. Don't have enough
        // info to recover.
        return{ tefEXCEPTION, false };
    }
    try
    {
        if (!preclaimResult.likelyToClaimFee)
            return{ preclaimResult.ter, false };
        ApplyContext ctx(app, view,
            preclaimResult.tx, preclaimResult.ter,
                preclaimResult.baseFee, preclaimResult.flags,
                    preclaimResult.j);
        return invoke_apply(ctx);
    }
    catch (std::exception const& e)
    {
        JLOG(preclaimResult.j.fatal()) <<
            "apply: " << e.what();
        return { tefEXCEPTION, false };
    }
}

void afterApply(ReadView const& view, STTx const& tx, TER terResult,
                Application& app, beast::Journal& j)
{
    switch (tx.getTxnType())
    {
    case ttPAYMENT: Payment::afterApply(view, tx, terResult, app, j); break;
    default: break;
    }
}

} //
