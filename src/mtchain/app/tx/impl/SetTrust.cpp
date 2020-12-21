//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/SetTrust.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/st.h>
#include <mtchain/ledger/View.h>

namespace mtchain {

TER
SetTrust::preflight (PreflightContext const& ctx)
{
    // JUST RETURN, FOR CANCEL ISSUE CURRENCY
    //return temBAD_AMOUNT;

    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    auto& tx = ctx.tx;
    auto& j = ctx.j;

    std::uint32_t const uTxFlags = tx.getFlags ();

    if (uTxFlags & tfTrustSetMask)
    {
        JLOG(j.trace()) <<
            "Malformed transaction: Invalid flags set.";
        return temINVALID_FLAG;
    }

    STAmount const saLimitAmount (tx.getFieldAmount (sfLimitAmount));

    if (!isLegalNet (saLimitAmount))
        return temBAD_AMOUNT;

    if (saLimitAmount.native ())
    {
        JLOG(j.trace()) <<
            "Malformed transaction: specifies native limit " <<
            saLimitAmount.getFullText ();
        return temBAD_LIMIT;
    }

    if (badCurrency() == saLimitAmount.getCurrency ())
    {
        JLOG(j.trace()) <<
            "Malformed transaction: specifies M as IOU";
        return temBAD_CURRENCY;
    }

    if (saLimitAmount < zero)
    {
        JLOG(j.trace()) <<
            "Malformed transaction: Negative credit limit.";
        return temBAD_LIMIT;
    }

    // Check if destination makes sense.
    auto const& issuer = saLimitAmount.getIssuer ();

    if (!issuer || issuer == noAccount())
    {
        JLOG(j.trace()) <<
            "Malformed transaction: no destination account.";
        return temDST_NEEDED;
    }

    return preflight2 (ctx);
}

TER
SetTrust::preclaim(PreclaimContext const& ctx)
{
    auto const id = ctx.tx[sfAccount];

    auto const sle = ctx.view.read(
        keylet::account(id));

    std::uint32_t const uTxFlags = ctx.tx.getFlags();

    bool const bSetAuth = (uTxFlags & tfSetfAuth);

    if (bSetAuth && !(sle->getFieldU32(sfFlags) & lsfRequireAuth))
    {
        JLOG(ctx.j.trace()) <<
            "Retry: Auth not required.";
        return tefNO_AUTH_REQUIRED;
    }

    auto const saLimitAmount = ctx.tx[sfLimitAmount];

    auto const currency = saLimitAmount.getCurrency();
    auto const uDstAccountID = saLimitAmount.getIssuer();

    if (id == uDstAccountID)
    {
        // Prevent trustline to self from being created,
        // unless one has somehow already been created
        // (in which case doApply will clean it up).
        auto const sleDelete = ctx.view.read(
            keylet::line(id, uDstAccountID, currency));

        if (!sleDelete)
        {
            JLOG(ctx.j.trace()) <<
                "Malformed transaction: Can not extend credit to self.";
            return temDST_IS_SRC;
        }
    }

    auto const sleIssue = ctx.view.read(keylet::account(uDstAccountID));
    if (!sleIssue)
    {
        return tecNO_ISSUER;
    }

    if (!(sleIssue->getFieldU32(sfFlags) & lsfDefaultMtchain))
    {
        return tecFORBID_TRUST_LINE;
    }

    return tesSUCCESS;
}

TER
SetTrust::doApply ()
{
    TER terResult = tesSUCCESS;

    STAmount const saLimitAmount (ctx_.tx.getFieldAmount (sfLimitAmount));
    bool const bQualityIn (ctx_.tx.isFieldPresent (sfQualityIn));
    bool const bQualityOut (ctx_.tx.isFieldPresent (sfQualityOut));

    Currency const currency (saLimitAmount.getCurrency ());
    AccountID uDstAccountID (saLimitAmount.getIssuer ());

    // true, iff current is high account.
    bool const bHigh = account_ > uDstAccountID;

    auto const sle = view().peek(
        keylet::account(account_));

    std::uint32_t const uOwnerCount = sle->getFieldU32 (sfOwnerCount);

    // The reserve that is required to create the line. Note
    // that although the reserve increases with every item
    // an account owns, in the case of trust lines we only
    // *enforce* a reserve if the user owns more than two
    // items.
    //
    // We do this because being able to exchange currencies,
    // which needs trust lines, is a powerful MTChain feature.
    // So we want to make it easy for a gateway to fund the
    // accounts of its users without fear of being tricked.
    //
    // Without this logic, a gateway that wanted to have a
    // new user use its services, would have to give that
    // user enough M to cover not only the account reserve
    // but the incremental reserve for the trust line as
    // well. A person with no intention of using the gateway
    // could use the extra M for their own purposes.

    MAmount const reserveCreate (/*(uOwnerCount < 2)
        ? MAmount (zero)
        : */view().fees().accountReserve(uOwnerCount + 1));

    std::uint32_t uQualityIn (bQualityIn ? ctx_.tx.getFieldU32 (sfQualityIn) : 0);
    std::uint32_t uQualityOut (bQualityOut ? ctx_.tx.getFieldU32 (sfQualityOut) : 0);

    if (bQualityOut && QUALITY_ONE == uQualityOut)
        uQualityOut = 0;

    std::uint32_t const uTxFlags = ctx_.tx.getFlags ();

    bool const bSetAuth = (uTxFlags & tfSetfAuth);
    bool const bSetNoMtchain = (uTxFlags & tfSetNoMtchain);
    bool const bClearNoMtchain  = (uTxFlags & tfClearNoMtchain);
    bool const bSetFreeze = (uTxFlags & tfSetFreeze);
    bool const bClearFreeze = (uTxFlags & tfClearFreeze);

    auto viewJ = ctx_.app.journal ("View");

    if (account_ == uDstAccountID)
    {
        // The only purpose here is to allow a mistakenly created
        // trust line to oneself to be deleted. If no such trust
        // lines exist now, why not remove this code and simply
        // return an error?
        SLE::pointer sleDelete = view().peek (
            keylet::line(account_, uDstAccountID, currency));

        JLOG(j_.warn()) <<
            "Clearing redundant line.";

        return trustDelete (view(),
            sleDelete, account_, uDstAccountID, viewJ);
    }

    SLE::pointer sleDst =
        view().peek (keylet::account(uDstAccountID));

    if (!sleDst)
    {
        JLOG(j_.trace()) <<
            "Delay transaction: Destination account does not exist.";
        return tecNO_DST;
    }

    STAmount saLimitAllow = saLimitAmount;
    saLimitAllow.setIssuer (account_);

    SLE::pointer sleMtchainState = view().peek (
        keylet::line(account_, uDstAccountID, currency));

    if (sleMtchainState)
    {
        STAmount        saLowBalance;
        STAmount        saLowLimit;
        STAmount        saHighBalance;
        STAmount        saHighLimit;
        std::uint32_t   uLowQualityIn;
        std::uint32_t   uLowQualityOut;
        std::uint32_t   uHighQualityIn;
        std::uint32_t   uHighQualityOut;
        auto const& uLowAccountID   = !bHigh ? account_ : uDstAccountID;
        auto const& uHighAccountID  =  bHigh ? account_ : uDstAccountID;
        SLE::ref        sleLowAccount   = !bHigh ? sle : sleDst;
        SLE::ref        sleHighAccount  =  bHigh ? sle : sleDst;

        //
        // Balances
        //

        saLowBalance    = sleMtchainState->getFieldAmount (sfBalance);
        saHighBalance   = -saLowBalance;

        //
        // Limits
        //

        sleMtchainState->setFieldAmount (!bHigh ? sfLowLimit : sfHighLimit, saLimitAllow);

        saLowLimit  = !bHigh ? saLimitAllow : sleMtchainState->getFieldAmount (sfLowLimit);
        saHighLimit =  bHigh ? saLimitAllow : sleMtchainState->getFieldAmount (sfHighLimit);

        //
        // Quality in
        //

        if (!bQualityIn)
        {
            // Not setting. Just get it.

            uLowQualityIn   = sleMtchainState->getFieldU32 (sfLowQualityIn);
            uHighQualityIn  = sleMtchainState->getFieldU32 (sfHighQualityIn);
        }
        else if (uQualityIn)
        {
            // Setting.

            sleMtchainState->setFieldU32 (!bHigh ? sfLowQualityIn : sfHighQualityIn, uQualityIn);

            uLowQualityIn   = !bHigh ? uQualityIn : sleMtchainState->getFieldU32 (sfLowQualityIn);
            uHighQualityIn  =  bHigh ? uQualityIn : sleMtchainState->getFieldU32 (sfHighQualityIn);
        }
        else
        {
            // Clearing.

            sleMtchainState->makeFieldAbsent (!bHigh ? sfLowQualityIn : sfHighQualityIn);

            uLowQualityIn   = !bHigh ? 0 : sleMtchainState->getFieldU32 (sfLowQualityIn);
            uHighQualityIn  =  bHigh ? 0 : sleMtchainState->getFieldU32 (sfHighQualityIn);
        }

        if (QUALITY_ONE == uLowQualityIn)   uLowQualityIn   = 0;

        if (QUALITY_ONE == uHighQualityIn)  uHighQualityIn  = 0;

        //
        // Quality out
        //

        if (!bQualityOut)
        {
            // Not setting. Just get it.

            uLowQualityOut  = sleMtchainState->getFieldU32 (sfLowQualityOut);
            uHighQualityOut = sleMtchainState->getFieldU32 (sfHighQualityOut);
        }
        else if (uQualityOut)
        {
            // Setting.

            sleMtchainState->setFieldU32 (!bHigh ? sfLowQualityOut : sfHighQualityOut, uQualityOut);

            uLowQualityOut  = !bHigh ? uQualityOut : sleMtchainState->getFieldU32 (sfLowQualityOut);
            uHighQualityOut =  bHigh ? uQualityOut : sleMtchainState->getFieldU32 (sfHighQualityOut);
        }
        else
        {
            // Clearing.

            sleMtchainState->makeFieldAbsent (!bHigh ? sfLowQualityOut : sfHighQualityOut);

            uLowQualityOut  = !bHigh ? 0 : sleMtchainState->getFieldU32 (sfLowQualityOut);
            uHighQualityOut =  bHigh ? 0 : sleMtchainState->getFieldU32 (sfHighQualityOut);
        }

        std::uint32_t const uFlagsIn (sleMtchainState->getFieldU32 (sfFlags));
        std::uint32_t uFlagsOut (uFlagsIn);

        if (bSetNoMtchain && !bClearNoMtchain && (bHigh ? saHighBalance : saLowBalance) >= zero)
        {
            uFlagsOut |= (bHigh ? lsfHighNoMtchain : lsfLowNoMtchain);
        }
        else if (bClearNoMtchain && !bSetNoMtchain)
        {
            uFlagsOut &= ~(bHigh ? lsfHighNoMtchain : lsfLowNoMtchain);
        }

        if (bSetFreeze && !bClearFreeze && !sle->isFlag  (lsfNoFreeze))
        {
            uFlagsOut           |= (bHigh ? lsfHighFreeze : lsfLowFreeze);
        }
        else if (bClearFreeze && !bSetFreeze)
        {
            uFlagsOut           &= ~(bHigh ? lsfHighFreeze : lsfLowFreeze);
        }

        if (QUALITY_ONE == uLowQualityOut)  uLowQualityOut  = 0;

        if (QUALITY_ONE == uHighQualityOut) uHighQualityOut = 0;

        bool const bLowDefMtchain        = sleLowAccount->getFlags() & lsfDefaultMtchain;
        bool const bHighDefMtchain       = sleHighAccount->getFlags() & lsfDefaultMtchain;

        bool const  bLowReserveSet      = uLowQualityIn || uLowQualityOut ||
                                            ((uFlagsOut & lsfLowNoMtchain) == 0) != bLowDefMtchain ||
                                            (uFlagsOut & lsfLowFreeze) ||
                                            saLowLimit || saLowBalance > zero;
        bool const  bLowReserveClear    = !bLowReserveSet;

        bool const  bHighReserveSet     = uHighQualityIn || uHighQualityOut ||
                                            ((uFlagsOut & lsfHighNoMtchain) == 0) != bHighDefMtchain ||
                                            (uFlagsOut & lsfHighFreeze) ||
                                            saHighLimit || saHighBalance > zero;
        bool const  bHighReserveClear   = !bHighReserveSet;

        bool const  bDefault            = bLowReserveClear && bHighReserveClear;

        bool const  bLowReserved = (uFlagsIn & lsfLowReserve);
        bool const  bHighReserved = (uFlagsIn & lsfHighReserve);

        bool        bReserveIncrease    = false;

        if (bSetAuth)
        {
            uFlagsOut |= (bHigh ? lsfHighAuth : lsfLowAuth);
        }

        if (bLowReserveSet && !bLowReserved)
        {
            // Set reserve for low account.
            adjustOwnerCount(view(),
                sleLowAccount, 1, viewJ);
            uFlagsOut |= lsfLowReserve;

            if (!bHigh)
                bReserveIncrease = true;
        }

        if (bLowReserveClear && bLowReserved)
        {
            // Clear reserve for low account.
            adjustOwnerCount(view(),
                sleLowAccount, -1, viewJ);
            uFlagsOut &= ~lsfLowReserve;
        }

        if (bHighReserveSet && !bHighReserved)
        {
            // Set reserve for high account.
            adjustOwnerCount(view(),
                sleHighAccount, 1, viewJ);
            uFlagsOut |= lsfHighReserve;

            if (bHigh)
                bReserveIncrease    = true;
        }

        if (bHighReserveClear && bHighReserved)
        {
            // Clear reserve for high account.
            adjustOwnerCount(view(),
                sleHighAccount, -1, viewJ);
            uFlagsOut &= ~lsfHighReserve;
        }

        if (uFlagsIn != uFlagsOut)
            sleMtchainState->setFieldU32 (sfFlags, uFlagsOut);

        if (bDefault || badCurrency() == currency)
        {
            // Delete.

            terResult = trustDelete (view(),
                sleMtchainState, uLowAccountID, uHighAccountID, viewJ);
        }
        // Reserve is not scaled by load.
        else if (bReserveIncrease && mPriorBalance < reserveCreate)
        {
            JLOG(j_.trace()) <<
                "Delay transaction: Insufficent reserve to add trust line.";

            // Another transaction could provide M to the account and then
            // this transaction would succeed.
            terResult = tecINSUF_RESERVE_LINE;
        }
        else
        {
            view().update (sleMtchainState);

            JLOG(j_.trace()) << "Modify mtchain line";
        }
    }
    // Line does not exist.
    else if (! saLimitAmount &&                          // Setting default limit.
        (! bQualityIn || ! uQualityIn) &&           // Not setting quality in or setting default quality in.
        (! bQualityOut || ! uQualityOut) &&         // Not setting quality out or setting default quality out.
        (! (view().rules().enabled(featureTrustSetAuth)) || ! bSetAuth))
    {
        JLOG(j_.trace()) <<
            "Redundant: Setting non-existent mtchain line to defaults.";
        return tecNO_LINE_REDUNDANT;
    }
    else if (mPriorBalance < reserveCreate) // Reserve is not scaled by load.
    {
        JLOG(j_.trace()) <<
            "Delay transaction: Line does not exist. Insufficent reserve to create line.";

        // Another transaction could create the account and then this transaction would succeed.
        terResult = tecNO_LINE_INSUF_RESERVE;
    }
    else
    {
        // Zero balance in currency.
        STAmount saBalance ({currency, noAccount()});

        uint256 index (getMtchainStateIndex (
            account_, uDstAccountID, currency));

        JLOG(j_.trace()) <<
            "doTrustSet: Creating mtchain line: " <<
            to_string (index);

        // Create a new mtchain line.
        terResult = trustCreate (view(),
            bHigh,
            account_,
            uDstAccountID,
            index,
            sle,
            bSetAuth,
            bSetNoMtchain && !bClearNoMtchain,
            bSetFreeze && !bClearFreeze,
            saBalance,
            saLimitAllow,       // Limit for who is being charged.
            uQualityIn,
            uQualityOut, viewJ);
    }

    return terResult;
}

}