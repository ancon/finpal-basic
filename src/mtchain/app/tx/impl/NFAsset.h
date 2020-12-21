//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_NFASSET_H_INCLUDED
#define MTCHAIN_TX_NFASSET_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>

namespace mtchain {

class CreateAsset : public Transactor
{
public:
    CreateAsset (ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static TER preflight (PreflightContext const& ctx);

    TER doApply () override;
};


class ApproveAsset : public Transactor
{
public:
    ApproveAsset (ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static TER preflight (PreflightContext const& ctx);

    static TER preclaim (PreclaimContext const& ctx);

    TER doApply () override;

protected:
    void preCompute() override
    {
        Transactor::preCompute();

        auto const& tx = ctx_.tx;
        assetAuthKey_ = keylet::nfasset(tx.getFieldH256(sfAssetID), account_,
                                        tx.getAccountID(sfDestination));
    }

    boost::optional<Keylet> assetAuthKey_;
};


class CancelApproveAsset : public ApproveAsset
{
public:
    CancelApproveAsset (ApplyContext& ctx) : ApproveAsset(ctx)
    {
    }

    TER doApply () override;
};


class DestroyAsset : public Transactor
{
public:
    DestroyAsset (ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static TER preflight (PreflightContext const& ctx);

    static TER preclaim (PreclaimContext const& ctx);

    TER doApply () override;
};


class SetAsset : public DestroyAsset
{
public:
    SetAsset (ApplyContext& ctx) : DestroyAsset(ctx)
    {
    }

    TER doApply () override;
};

std::shared_ptr<SLE> addAssetOwner(AccountID const& owner, uint256 const& assetid,
				   ApplyView &view, beast::Journal const& j);
void removeAssetOwner(AccountID const& owner, std::shared_ptr<SLE> sleAssetOwner, ApplyView &view);

} //

#endif
