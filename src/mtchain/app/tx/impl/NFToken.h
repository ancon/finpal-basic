//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_NFTOKEN_H_INCLUDED
#define MTCHAIN_TX_NFTOKEN_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/protocol/TxFlags.h>

namespace mtchain {

class CreateToken : public Transactor
{
public:
    CreateToken (ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static TER preflight (PreflightContext const& ctx);

    static TER preclaim (PreclaimContext const& ctx);

    TER doApply () override;
};


class DestroyToken : public Transactor
{
public:
    DestroyToken (ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static TER preflight (PreflightContext const& ctx);

    TER doApply () override;

protected:
    virtual bool checkPermission (std::shared_ptr<SLE> sleToken, std::shared_ptr<SLE> sleAsset)
    {
        if (sleToken->getAccountID(sfOwner) == account_)
        {
            if ((sleAsset->getFieldU32(sfFlags) & tfDestroyToken))
                return true;
        }

        return false;
    }

    void preCompute() override
    {
        Transactor::preCompute();

        auto const& tx = ctx_.tx;
        tokenKey_ = tx.isFieldPresent(sfTokenID) ? keylet::nftoken(tx.getFieldH256(sfTokenID))
                         : keylet::nftoken(tx.getFieldH256(sfAssetID), tx.getFieldVL(sfIdent));
    }

    boost::optional<Keylet> tokenKey_;
};


class RevokeToken : public DestroyToken
{
public:
    RevokeToken (ApplyContext& ctx) : DestroyToken(ctx)
    {
    }

protected:
    bool checkPermission (std::shared_ptr<SLE> sleToken, std::shared_ptr<SLE> sleAsset) override
    {
        if (sleAsset->getAccountID(sfIssuer) == account_)
        {
            if ((sleAsset->getFieldU32(sfFlags) & tfRevokeToken))
                return true;
        }

        return false;
    }
};


class TransferToken : public DestroyToken
{
public:
    TransferToken (ApplyContext& ctx) : DestroyToken(ctx)
    {
    }

    static TER preclaim (PreclaimContext const& ctx);

    TER doApply () override;

protected:
    bool checkPermission (std::shared_ptr<SLE> sleToken, std::shared_ptr<SLE> sleAsset) override
    {
        auto const& owner = sleToken->getAccountID(sfOwner);
        auto pass = false;

        if (owner == account_)
            pass = true;
        else if (sleToken->isFieldPresent(sfApproved) &&
                 sleToken->getAccountID(sfApproved) == account_)
            pass = true;
        else
        {
            auto const& k = keylet::nfasset(sleToken->getFieldH256(sfAssetID), owner, account_);
            if (view().peek(k)) pass = true;
        }

        if (pass)
        {
            if ((sleAsset->getFieldU32(sfFlags) & tfTransferToken))
                return true;
        }

        return false;
    }
};


class ApproveToken : public TransferToken
{
public:
    ApproveToken (ApplyContext& ctx) : TransferToken(ctx)
    {
    }

    TER doApply () override;

protected:
    bool checkPermission (std::shared_ptr<SLE> sleToken,
                          std::shared_ptr<SLE> sleAsset = {}) override
    {
        return sleToken->getAccountID(sfOwner) == account_;
    }
};


class CancelApproveToken : public DestroyToken
{
public:
    CancelApproveToken (ApplyContext& ctx) : DestroyToken(ctx)
    {
    }

    TER doApply () override;

protected:
    bool checkPermission (std::shared_ptr<SLE> sleToken,
                          std::shared_ptr<SLE> sleAsset = {}) override
    {
        return sleToken->getAccountID(sfOwner) == account_;
    }
};


class SetToken : public DestroyToken
{
public:
    SetToken (ApplyContext& ctx) : DestroyToken(ctx)
    {
    }

    TER doApply () override;

protected:
    virtual bool checkPermission (std::shared_ptr<SLE> sleToken,
                                  std::shared_ptr<SLE> sleAsset = {})
    {
        auto const& owner = sleToken->getAccountID(sfOwner);
        if (sleToken->getAccountID(sfOwner) == account_)
        {
            return true;
        }
        else if (sleToken->isFieldPresent(sfApproved) &&
                 sleToken->getAccountID(sfApproved) == account_)
        {
            return true;
        }
        else
        {
            auto const& k = keylet::nfasset(sleToken->getFieldH256(sfAssetID), owner, account_);
            if (view().peek(k)) return true;
        }

        return false;
    }
};

} //

#endif
