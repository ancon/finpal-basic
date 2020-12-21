//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/SetRegularKey.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/TxFlags.h>
#include <mtchain/protocol/types.h>

namespace mtchain {

std::uint64_t
SetRegularKey::calculateBaseFee (
    PreclaimContext const& ctx)
{
    auto const id = ctx.tx.getAccountID(sfAccount);
    auto const spk = ctx.tx.getSigningPubKey();

    if (publicKeyType (makeSlice (spk)))
    {
        if (calcAccountID(PublicKey (makeSlice(spk))) == id)
        {
            auto const sle = ctx.view.read(keylet::account(id));

            if (sle && (! (sle->getFlags () & lsfPasswordSpent)))
            {
                // flag is armed and they signed with the right account
                return 0;
            }
        }
    }

    return Transactor::calculateBaseFee (ctx);
}

TER
SetRegularKey::preflight (PreflightContext const& ctx)
{
    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    std::uint32_t const uTxFlags = ctx.tx.getFlags ();

    if (uTxFlags & tfUniversalMask)
    {
        JLOG(ctx.j.trace()) <<
            "Malformed transaction: Invalid flags set.";

        return temINVALID_FLAG;
    }

    return preflight2(ctx);
}

TER
SetRegularKey::doApply ()
{
    auto const sle = view().peek(
        keylet::account(account_));

    if (mFeeDue == zero)
        sle->setFlag (lsfPasswordSpent);

    if (ctx_.tx.isFieldPresent (sfRegularKey))
    {
        sle->setAccountID (sfRegularKey,
            ctx_.tx.getAccountID (sfRegularKey));
    }
    else
    {
        if (sle->isFlag (lsfDisableMaster) &&
            !view().peek (keylet::signers (account_)))
            // Account has disabled master key and no multi-signer signer list.
            return tecNO_ALTERNATIVE_KEY;

        sle->makeFieldAbsent (sfRegularKey);
    }

    return tesSUCCESS;
}

}
