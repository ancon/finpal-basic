//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/Credit.h>
#include <mtchain/app/paths/impl/AmountSpec.h>
#include <mtchain/app/paths/impl/Steps.h>
#include <mtchain/app/paths/impl/StepChecks.h>
#include <mtchain/basics/Log.h>
#include <mtchain/ledger/PaymentSandbox.h>
#include <mtchain/protocol/IOUAmount.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/MAmount.h>

#include <boost/container/flat_set.hpp>

#include <numeric>
#include <sstream>

namespace mtchain {

class MEndpointStep : public StepImp<MAmount, MAmount, MEndpointStep>
{
  private:
    AccountID acc_;
    bool isLast_;
    beast::Journal j_;

    // Since this step will always be an endpoint in a strand
    // (either the first or last step) the same cache is used
    // for cachedIn and cachedOut and only one will ever be used
    boost::optional<MAmount> cache_;

    boost::optional<EitherAmount>
    cached () const
    {
        if (!cache_)
            return boost::none;
        return EitherAmount (*cache_);
    }
  public:
    MEndpointStep (
        AccountID const& acc,
        bool isLast,
        beast::Journal j)
            :acc_(acc)
            , isLast_(isLast)
            , j_ (j) {}

    AccountID const& acc () const
    {
        return acc_;
    };

    boost::optional<EitherAmount>
    cachedIn () const override
    {
        return cached ();
    }

    boost::optional<EitherAmount>
    cachedOut () const override
    {
        return cached ();
    }

    std::pair<MAmount, MAmount>
    revImp (
        PaymentSandbox& sb,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        MAmount const& out);

    std::pair<MAmount, MAmount>
    fwdImp (
        PaymentSandbox& sb,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        MAmount const& in);

    std::pair<bool, EitherAmount>
    validFwd (
        PaymentSandbox& sb,
        ApplyView& afView,
        EitherAmount const& in) override;

    // Check for errors and violations of frozen constraints.
    TER check (StrandContext const& ctx) const;

private:
    friend bool operator==(MEndpointStep const& lhs, MEndpointStep const& rhs);

    friend bool operator!=(MEndpointStep const& lhs, MEndpointStep const& rhs)
    {
        return ! (lhs == rhs);
    }

    bool equal (Step const& rhs) const override
    {
        if (auto ds = dynamic_cast<MEndpointStep const*> (&rhs))
        {
            return *this == *ds;
        }
        return false;
    }

    std::string logString () const override
    {
        std::ostringstream ostr;
        ostr <<
            "MEndpointStep: " <<
            "\nAcc: " << acc_;
        return ostr.str ();
    }
};

inline bool operator==(MEndpointStep const& lhs, MEndpointStep const& rhs)
{
    return lhs.acc_ == rhs.acc_ && lhs.isLast_ == rhs.isLast_;
}

static
MAmount
mLiquid (ReadView& sb, AccountID const& src)
{
    return accountHolds(
        sb, src, mCurrency(), mAccount(), fhIGNORE_FREEZE, {}).m();
}


std::pair<MAmount, MAmount>
MEndpointStep::revImp (
    PaymentSandbox& sb,
    ApplyView& afView,
    boost::container::flat_set<uint256>& ofrsToRm,
    MAmount const& out)
{
    auto const balance = mLiquid (sb, acc_);
    auto const result = isLast_ ? out : std::min (balance, out);

    auto& sender = isLast_ ? mAccount() : acc_;
    auto& receiver = isLast_ ? acc_ : mAccount();
    auto ter   = accountSend (sb, sender, receiver, toSTAmount (result), j_);
    if (ter != tesSUCCESS)
        return {MAmount{beast::zero}, MAmount{beast::zero}};

    cache_.emplace (result);
    return {result, result};
}

std::pair<MAmount, MAmount>
MEndpointStep::fwdImp (
    PaymentSandbox& sb,
    ApplyView& afView,
    boost::container::flat_set<uint256>& ofrsToRm,
    MAmount const& in)
{
    assert (cache_);
    auto const balance = mLiquid (sb, acc_);
    auto const result = isLast_ ? in : std::min (balance, in);

    auto& sender = isLast_ ? mAccount() : acc_;
    auto& receiver = isLast_ ? acc_ : mAccount();
    auto ter   = accountSend (sb, sender, receiver, toSTAmount (result), j_);
    if (ter != tesSUCCESS)
        return {MAmount{beast::zero}, MAmount{beast::zero}};

    cache_.emplace (result);
    return {result, result};
}

std::pair<bool, EitherAmount>
MEndpointStep::validFwd (
    PaymentSandbox& sb,
    ApplyView& afView,
    EitherAmount const& in)
{
    if (!cache_)
    {
        JLOG (j_.error()) << "Expected valid cache in validFwd";
        return {false, EitherAmount (MAmount (beast::zero))};
    }

    assert (in.native);

    auto const& mIn = in.m;
    auto const balance = mLiquid (sb, acc_);

    if (!isLast_ && balance < mIn)
    {
        JLOG (j_.error()) << "MEndpointStep: Strand re-execute check failed."
            << " Insufficient balance: " << to_string (balance)
            << " Requested: " << to_string (mIn);
        return {false, EitherAmount (balance)};
    }

    if (mIn != *cache_)
    {
        JLOG (j_.error()) << "MEndpointStep: Strand re-execute check failed."
            << " ExpectedIn: " << to_string (*cache_)
            << " CachedIn: " << to_string (mIn);
    }
    return {true, in};
}

TER
MEndpointStep::check (StrandContext const& ctx) const
{
    if (!acc_)
    {
        JLOG (j_.debug()) << "MEndpointStep: specified bad account.";
        return temBAD_PATH;
    }

    auto sleAcc = ctx.view.read (keylet::account (acc_));
    if (!sleAcc)
    {
        JLOG (j_.warn()) << "MEndpointStep: can't send or receive Ms from "
                             "non-existent account: "
                          << acc_;
        return terNO_ACCOUNT;
    }

    if (!ctx.isFirst && !ctx.isLast)
    {
        return temBAD_PATH;
    }

    auto& src = isLast_ ? mAccount () : acc_;
    auto& dst = isLast_ ? acc_ : mAccount();
    auto ter = checkFreeze (ctx.view, src, dst, mCurrency ());
    if (ter != tesSUCCESS)
        return ter;

    return tesSUCCESS;
}

//------------------------------------------------------------------------------

namespace test
{
// Needed for testing
bool mEndpointStepEqual (Step const& step, AccountID const& acc)
{
    if (auto xs = dynamic_cast<MEndpointStep const*> (&step))
    {
        return xs->acc () == acc;
    }
    return false;
}
}

//------------------------------------------------------------------------------

std::pair<TER, std::unique_ptr<Step>>
make_MEndpointStep (
    StrandContext const& ctx,
    AccountID const& acc)
{
    auto r = std::make_unique<MEndpointStep> (acc, ctx.isLast, ctx.j);
    auto ter = r->check (ctx);
    if (ter != tesSUCCESS)
        return {ter, nullptr};
    return {tesSUCCESS, std::move (r)};
}

} //
