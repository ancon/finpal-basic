//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/CreateTicket.h>
#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

TER
CreateTicket::preflight (PreflightContext const& ctx)
{
    if (! ctx.rules.enabled(featureTickets))
        return temDISABLED;

    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    if (ctx.tx.isFieldPresent (sfExpiration))
    {
        if (ctx.tx.getFieldU32 (sfExpiration) == 0)
        {
            JLOG(ctx.j.warn()) <<
                "Malformed transaction: bad expiration";
            return temBAD_EXPIRATION;
        }
    }

    return preflight2 (ctx);
}

TER
CreateTicket::doApply ()
{
    auto const sle = view().peek(keylet::account(account_));

    // A ticket counts against the reserve of the issuing account, but we
    // check the starting balance because we want to allow dipping into the
    // reserve to pay fees.
    {
        auto const reserve = view().fees().accountReserve(
            sle->getFieldU32(sfOwnerCount) + 1);

        if (mPriorBalance < reserve)
            return tecINSUFFICIENT_RESERVE;
    }

    NetClock::time_point expiration{};

    if (ctx_.tx.isFieldPresent (sfExpiration))
    {
        expiration = NetClock::time_point(NetClock::duration(ctx_.tx[sfExpiration]));

        if (view().parentCloseTime() >= expiration)
            return tesSUCCESS;
    }

    SLE::pointer sleTicket = std::make_shared<SLE>(ltTICKET,
        getTicketIndex (account_, ctx_.tx.getSequence ()));
    sleTicket->setAccountID (sfAccount, account_);
    sleTicket->setFieldU32 (sfSequence, ctx_.tx.getSequence ());
    if (expiration != NetClock::time_point{})
        sleTicket->setFieldU32 (sfExpiration, expiration.time_since_epoch().count());
    view().insert (sleTicket);

    if (ctx_.tx.isFieldPresent (sfTarget))
    {
        AccountID const target_account (ctx_.tx.getAccountID (sfTarget));

        SLE::pointer sleTarget = view().peek (keylet::account(target_account));

        // Destination account does not exist.
        if (!sleTarget)
            return tecNO_TARGET;

        // The issuing account is the default account to which the ticket
        // applies so don't bother saving it if that's what's specified.
        if (target_account != account_)
            sleTicket->setAccountID (sfTarget, target_account);
    }

    std::uint64_t hint;

    auto viewJ = ctx_.app.journal ("View");

    auto result = dirAdd(view(), hint, keylet::ownerDir (account_),
        sleTicket->key(), describeOwnerDir (account_), viewJ);

    JLOG(j_.trace()) <<
        "Creating ticket " << to_string (sleTicket->key()) <<
        ": " << transHuman (result.first);

    if (result.first == tesSUCCESS)
    {
        sleTicket->setFieldU64(sfOwnerNode, hint);

        // If we succeeded, the new entry counts agains the
        // creator's reserve.
        adjustOwnerCount(view(), sle, 1, viewJ);
    }

    return result.first;
}

}
