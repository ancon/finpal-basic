//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.
    Permission to use, copy, modify, and/or distribute this software for any
*/
//==============================================================================

#include <BeastConfig.h>
#include <test/jtx.h>
#include <mtchain/app/paths/Flow.h>
#include <mtchain/app/paths/impl/Steps.h>
#include <mtchain/basics/contract.h>
#include <mtchain/core/Config.h>
#include <mtchain/ledger/PaymentSandbox.h>
#include <mtchain/ledger/Sandbox.h>
#include <test/jtx/PathSet.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/JsonFields.h>
namespace mtchain {
namespace test {

struct Flow_test;

struct DirectStepInfo
{
    AccountID src;
    AccountID dst;
    Currency currency;
};

struct MEndpointStepInfo
{
    AccountID acc;
};

enum class TrustFlag {freeze, auth};

/*constexpr*/ std::uint32_t trustFlag (TrustFlag f, bool useHigh)
{
    switch(f)
    {
        case TrustFlag::freeze:
            if (useHigh)
                return lsfHighFreeze;
            return lsfLowFreeze;
        case TrustFlag::auth:
            if (useHigh)
                return lsfHighAuth;
            return lsfLowAuth;
    }
    return 0; // Silence warning about end of non-void function
}

bool getTrustFlag (jtx::Env const& env,
    jtx::Account const& src,
    jtx::Account const& dst,
    Currency const& cur,
    TrustFlag flag)
{
    if (auto sle = env.le (keylet::line (src, dst, cur)))
    {
        auto const useHigh = src.id() > dst.id();
        return sle->isFlag (trustFlag (flag, useHigh));
    }
    Throw<std::runtime_error> ("No line in getTrustFlag");
    return false; // silence warning
}

jtx::PrettyAmount
mMinusFee (jtx::Env const& env, std::int64_t mAmount)
{
    using namespace jtx;
    auto feeDrops = env.current ()->fees ().base;
    return drops (
        dropsPerM<std::int64_t>::value * mAmount - feeDrops);
};

bool equal (std::unique_ptr<Step> const& s1,
    DirectStepInfo const& dsi)
{
    if (!s1)
        return false;
    return test::directStepEqual (*s1, dsi.src, dsi.dst, dsi.currency);
}

bool equal (std::unique_ptr<Step> const& s1,
            MEndpointStepInfo const& msi)
{
    if (!s1)
        return false;
    return test::mEndpointStepEqual (*s1, msi.acc);
}

bool equal (std::unique_ptr<Step> const& s1, mtchain::Book const& bsi)
{
    if (!s1)
        return false;
    return bookStepEqual (*s1, bsi);
}

template <class Iter>
bool strandEqualHelper (Iter i)
{
    // base case. all args processed and found equal.
    return true;
}

template <class Iter, class StepInfo, class... Args>
bool strandEqualHelper (Iter i, StepInfo&& si, Args&&... args)
{
    if (!equal (*i, std::forward<StepInfo> (si)))
        return false;
    return strandEqualHelper (++i, std::forward<Args> (args)...);
}

template <class... Args>
bool equal (Strand const& strand, Args&&... args)
{
    if (strand.size () != sizeof...(Args))
        return false;
    if (strand.empty ())
        return true;
    return strandEqualHelper (strand.begin (), std::forward<Args> (args)...);
}

struct Flow_test : public beast::unit_test::suite
{
    // Account path element
    static auto APE(AccountID const& a)
    {
        return STPathElement (
            STPathElement::typeAccount, a, mCurrency (), mAccount ());
    };

    // Issue path element
    static auto IPE(Issue const& iss)
    {
        return STPathElement (
            STPathElement::typeCurrency | STPathElement::typeIssuer,
            mAccount (), iss.currency, iss.account);
    };

    // Issuer path element
    static auto IAPE(AccountID const& account)
    {
        return STPathElement (
            STPathElement::typeIssuer,
            mAccount (), mCurrency (), account);
    };

    // Currency path element
    static auto CPE(Currency const& c)
    {
        return STPathElement (
            STPathElement::typeCurrency, mAccount (), c, mAccount ());
    };

    void testToStrand ()
    {
        testcase ("To Strand");

        using namespace jtx;
        auto const alice = Account ("alice");
        auto const bob = Account ("bob");
        auto const carol = Account ("carol");
        auto const gw = Account ("gw");

        auto const USD = gw["USD"];
        auto const EUR = gw["EUR"];

        auto const eurC = EUR.currency;
        auto const usdC = USD.currency;

        using D = DirectStepInfo;
        using B = mtchain::Book;
        using MS = MEndpointStepInfo;

        auto test = [&, this](jtx::Env& env, Issue const& deliver,
            boost::optional<Issue> const& sendMaxIssue, STPath const& path,
            TER expTer, auto&&... expSteps)
        {
            auto r = toStrand (*env.current (), alice, bob,
                deliver, sendMaxIssue, path, true, env.app ().logs ().journal ("Flow"));
            BEAST_EXPECT(r.first == expTer);
            if (sizeof...(expSteps))
                BEAST_EXPECT(equal (
                    r.second, std::forward<decltype (expSteps)> (expSteps)...));
        };

        {
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, carol, gw);

            test (env, USD, boost::none, STPath(), terNO_LINE);

            env.trust (USD (1000), alice, bob, carol);
            test (env, USD, boost::none, STPath(), tecPATH_DRY);

            env (pay (gw, alice, USD (100)));
            env (pay (gw, carol, USD (100)));

            // Insert implied account
            test (env, USD, boost::none, STPath(), tesSUCCESS,
                D{alice, gw, usdC}, D{gw, bob, usdC});
            env.trust (EUR (1000), alice, bob);

            // Insert implied offer
            test (env, EUR, USD.issue (), STPath(), tesSUCCESS,
                D{alice, gw, usdC}, B{USD, EUR}, D{gw, bob, eurC});

            // Path with explicit offer
            test (env, EUR, USD.issue (), STPath ({IPE (EUR)}),
                tesSUCCESS, D{alice, gw, usdC}, B{USD, EUR}, D{gw, bob, eurC});

            // Path with offer that changes issuer only
            env.trust (carol["USD"] (1000), bob);
            test (env, carol["USD"], USD.issue (), STPath ({IAPE (carol)}),
                  tesSUCCESS,
                  D{alice, gw, usdC}, B{USD, carol["USD"]}, D{carol, bob, usdC});

            // Path with M src currency
            test (env, USD, mIssue (), STPath ({IPE (USD)}), tesSUCCESS,
                MS{alice}, B{M, USD}, D{gw, bob, usdC});

            // Path with M dst currency
            test (env, mIssue(), USD.issue (), STPath ({IPE (M)}),
                  tesSUCCESS, D{alice, gw, usdC}, B{USD, M}, MS{bob});

            // Path with M cross currency bridged payment
            test (env, EUR, USD.issue (), STPath ({CPE (mCurrency ())}),
                  tesSUCCESS,
                  D{alice, gw, usdC}, B{USD, M}, B{M, EUR}, D{gw, bob, eurC});

            // M -> M transaction can't include a path
            test (env, M, boost::none, STPath ({APE (carol)}), temBAD_PATH);

            {
                // The root account can't be the src or dst
                auto flowJournal = env.app ().logs ().journal ("Flow");
                {
                    // The root account can't be the dst
                    auto r = toStrand (*env.current (), alice,
                        mAccount (), M, USD.issue (), STPath (), true, flowJournal);
                    BEAST_EXPECT(r.first == temBAD_PATH);
                }
                {
                    // The root account can't be the src
                    auto r =
                        toStrand (*env.current (), mAccount (),
                            alice, M, boost::none, STPath (), true, flowJournal);
                    BEAST_EXPECT(r.first == temBAD_PATH);
                }
                {
                    // The root account can't be the src
                    auto r = toStrand (*env.current (),
                        noAccount (), bob, USD, boost::none, STPath (), true, flowJournal);
                    BEAST_EXPECT(r.first == terNO_ACCOUNT);
                }
            }

            // Create an offer with the same in/out issue
            test (env, EUR, USD.issue (), STPath ({IPE (USD), IPE (EUR)}),
                  temBAD_PATH);

            // Path element with type zero
            test (env, USD, boost::none,
                STPath ({STPathElement (
                    0, mAccount (), mCurrency (), mAccount ())}),
                temBAD_PATH);

            // The same account can't appear more than once on a path
            // `gw` will be used from alice->carol and implied between carol
            // and bob
            test (env, USD, boost::none, STPath ({APE (gw), APE (carol)}),
                temBAD_PATH_LOOP);

            // The same offer can't appear more than once on a path
            test (env, EUR, USD.issue (), STPath ({IPE (EUR), IPE (USD), IPE (EUR)}),
                  temBAD_PATH_LOOP);
        }

        {
            // cannot have more than one offer with the same output issue

            using namespace jtx;
            Env env (*this, features (featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (10000), alice, bob, carol);
            env.trust (EUR (10000), alice, bob, carol);

            env (pay (gw, bob, USD (100)));
            env (pay (gw, bob, EUR (100)));

            env (offer (bob, M (100), USD (100)));
            env (offer (bob, USD (100), EUR (100)), txflags (tfPassive));
            env (offer (bob, EUR (100), USD (100)), txflags (tfPassive));

            // payment path: M -> M/USD -> USD/EUR -> EUR/USD
            env (pay (alice, carol, USD (100)), path (~USD, ~EUR, ~USD),
                sendmax (M (200)), txflags (tfNoMTChainDirect),
                ter (temBAD_PATH_LOOP));
        }

        {
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, nomtchain (gw));
            env.trust (USD (1000), alice, bob);
            env (pay (gw, alice, USD (100)));
            test (env, USD, boost::none, STPath (), terNO_MTCHAIN);
        }

        {
            // check global freeze
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, gw);
            env.trust (USD (1000), alice, bob);
            env (pay (gw, alice, USD (100)));

            // Account can still issue payments
            env(fset(alice, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), tesSUCCESS);
            env(fclear(alice, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), tesSUCCESS);

            // Account can not issue funds
            env(fset(gw, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), terNO_LINE);
            env(fclear(gw, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), tesSUCCESS);

            // Account can not receive funds
            env(fset(bob, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), terNO_LINE);
            env(fclear(bob, asfGlobalFreeze));
            test (env, USD, boost::none, STPath (), tesSUCCESS);
        }
        {
            // Freeze between gw and alice
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, gw);
            env.trust (USD (1000), alice, bob);
            env (pay (gw, alice, USD (100)));
            test (env, USD, boost::none, STPath (), tesSUCCESS);
            env (trust (gw, alice["USD"] (0), tfSetFreeze));
            BEAST_EXPECT(getTrustFlag (env, gw, alice, usdC, TrustFlag::freeze));
            test (env, USD, boost::none, STPath (), terNO_LINE);
        }
        {
            // check no auth
            // An account may require authorization to receive IOUs from an
            // issuer
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, gw);
            env (fset (gw, asfRequireAuth));
            env.trust (USD (1000), alice, bob);
            // Authorize alice but not bob
            env (trust (gw, alice ["USD"] (1000), tfSetfAuth));
            BEAST_EXPECT(getTrustFlag (env, gw, alice, usdC, TrustFlag::auth));
            env (pay (gw, alice, USD (100)));
            env.require (balance (alice, USD (100)));
            test (env, USD, boost::none, STPath (), terNO_AUTH);

            // Check pure issue redeem still works
            auto r = toStrand (*env.current (), alice, gw, USD,
                boost::none, STPath (), true, env.app ().logs ().journal ("Flow"));
            BEAST_EXPECT(r.first == tesSUCCESS);
            BEAST_EXPECT(equal (r.second, D{alice, gw, usdC}));
        }
        {
            // Check path with sendMax and node with correct sendMax already set
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            env.fund (M (10000), alice, bob, gw);
            env.trust (USD (1000), alice, bob);
            env.trust (EUR (1000), alice, bob);
            env (pay (gw, alice, EUR (100)));
            auto const path = STPath ({STPathElement (STPathElement::typeAll,
                EUR.account, EUR.currency, EUR.account)});
            test (env, USD, EUR.issue(), path, tesSUCCESS);
        }
    }
    void testDirectStep ()
    {
        testcase ("Direct Step");

        using namespace jtx;
        auto const alice = Account ("alice");
        auto const bob = Account ("bob");
        auto const carol = Account ("carol");
        auto const dan = Account ("dan");
        auto const erin = Account ("erin");
        auto const UM = alice["USD"];
        auto const USDB = bob["USD"];
        auto const USDC = carol["USD"];
        auto const USDD = dan["USD"];
        auto const gw = Account ("gw");
        auto const USD = gw["USD"];
        {
            // Pay USD, trivial path
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, gw);
            env.trust (USD (1000), alice, bob);
            env (pay (gw, alice, USD (100)));
            env (pay (alice, bob, USD (10)), paths (USD));
            env.require (balance (bob, USD (10)));
        }
        {
            // M transfer
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob);
            env (pay (alice, bob, M (100)));
            env.require (balance (bob, M (10000 + 100)));
            env.require (balance (alice, mMinusFee (env, 10000 - 100)));
        }
        {
            // Partial payments
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, gw);
            env.trust (USD (1000), alice, bob);
            env (pay (gw, alice, USD (100)));
            env (pay (alice, bob, USD (110)), paths (USD),
                ter (tecPATH_PARTIAL));
            env.require (balance (bob, USD (0)));
            env (pay (alice, bob, USD (110)), paths (USD),
                txflags (tfPartialPayment));
            env.require (balance (bob, USD (100)));
        }
        {
            // Pay by rippling through accounts, use path finder
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, dan);
            env.trust (UM (10), bob);
            env.trust (USDB (10), carol);
            env.trust (USDC (10), dan);
            env (pay (alice, dan, USDC (10)), paths (UM));
            env.require (
                balance (bob, UM (10)),
                balance (carol, USDB (10)),
                balance (dan, USDC (10)));
        }
        {
            // Pay by rippling through accounts, specify path
            // and charge a transfer fee
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, dan);
            env.trust (UM (10), bob);
            env.trust (USDB (10), alice, carol);
            env.trust (USDC (10), dan);
            env (rate (bob, 1.1));

            // alice will redeem to bob; a transfer fee will be charged
            env (pay (bob, alice, USDB(6)));
            env (pay (alice, dan, USDC (5)), path (bob, carol),
                sendmax (UM (6)), txflags (tfNoMTChainDirect));
            env.require (balance (dan, USDC (5)));
            env.require (balance (alice, USDB (0.5)));
        }
        {
            // Pay by rippling through accounts, specify path and transfer fee
            // Test that the transfer fee is not charged when alice issues
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, dan);
            env.trust (UM (10), bob);
            env.trust (USDB (10), alice, carol);
            env.trust (USDC (10), dan);
            env (rate (bob, 1.1));

            env (pay (alice, dan, USDC (5)), path (bob, carol),
                 sendmax (UM (6)), txflags (tfNoMTChainDirect));
            env.require (balance (dan, USDC (5)));
            env.require (balance (bob, UM (5)));
        }
        {
            // test best quality path is taken
            // Paths: A->B->D->E ; A->C->D->E
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, dan, erin);
            env.trust (UM (10), bob, carol);
            env.trust (USDB (10), dan);
            env.trust (USDC (10), alice, dan);
            env.trust (USDD (20), erin);
            env (rate (bob, 1));
            env (rate (carol, 1.1));

            // Pay alice so she redeems to carol and a transfer fee is charged
            env (pay (carol, alice, USDC(10)));
            env (pay (alice, erin, USDD (5)), path (carol, dan),
                path (bob, dan), txflags (tfNoMTChainDirect));

            env.require (balance (erin, USDD (5)));
            env.require (balance (dan, USDB (5)));
            env.require (balance (dan, USDC (0)));
        }
        {
            // Limit quality
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol);
            env.trust (UM (10), bob);
            env.trust (USDB (10), carol);

            env (pay (alice, carol, USDB (5)), sendmax (UM (4)),
                txflags (tfLimitQuality | tfPartialPayment), ter (tecPATH_DRY));
            env.require (balance (carol, USDB (0)));

            env (pay (alice, carol, USDB (5)), sendmax (UM (4)),
                txflags (tfPartialPayment));
            env.require (balance (carol, USDB (4)));
        }
    }

    void testLineQuality ()
    {
        testcase ("Line Quality");

        using namespace jtx;
        auto const alice = Account ("alice");
        auto const bob = Account ("bob");
        auto const carol = Account ("carol");
        auto const dan = Account ("dan");
        auto const UM = alice["USD"];
        auto const USDB = bob["USD"];
        auto const USDC = carol["USD"];
        auto const USDD = dan["USD"];

        //   Dan -> Bob -> Alice -> Carol; vary bobDanQIn and bobAliceQOut
        for (auto bobDanQIn : {80, 100, 120})
            for (auto bobAliceQOut : {80, 100, 120})
                for (auto const& f : {feature ("nullFeature"), featureFlow})
                {
                    if (f != featureFlow && bobDanQIn < 100 && bobAliceQOut < 100)
                        continue;  // Bug in flow v1
                    Env env (*this, features (f));
                    env.fund (M (10000), alice, bob, carol, dan);
                    env (trust (bob, USDD (100)), qualityInPercent (bobDanQIn));
                    env (trust (bob, UM (100)),
                        qualityOutPercent (bobAliceQOut));
                    env (trust (carol, UM (100)));

                    env (pay (alice, bob, UM (100)));
                    env.require (balance (bob, UM (100)));
                    env (pay (dan, carol, UM (10)), path (bob),
                        sendmax (USDD (100)), txflags (tfNoMTChainDirect));
                    env.require (balance (bob, UM (90)));
                    if (bobAliceQOut > bobDanQIn)
                        env.require (
                            balance (bob, USDD (10.0 * double(bobAliceQOut) /
                                              double(bobDanQIn))));
                    else
                        env.require (balance (bob, USDD (10)));
                    env.require (balance (carol, UM (10)));
                }

        // bob -> alice -> carol; vary carolAliceQIn
        for (auto carolAliceQIn : {80, 100, 120})
            for (auto const& f : {feature ("nullFeature"), featureFlow})
            {
                Env env (*this, features (f));
                env.fund (M (10000), alice, bob, carol);
                env (trust (bob, UM (10)));
                env (trust (carol, UM (10)), qualityInPercent (carolAliceQIn));

                env (pay (alice, bob, UM (10)));
                env.require (balance (bob, UM (10)));
                env (pay (bob, carol, UM (5)), sendmax (UM (10)));
                auto const effectiveQ =
                    carolAliceQIn > 100 ? 1.0 : carolAliceQIn / 100.0;
                env.require (balance (bob, UM (10.0 - 5.0 / effectiveQ)));
            }

        // bob -> alice -> carol; bobAliceQOut varies.
        for (auto bobAliceQOut : {80, 100, 120})
            for (auto const& f : {feature ("nullFeature"), featureFlow})
            {
                Env env (*this, features (f));
                env.fund (M (10000), alice, bob, carol);
                env (trust (bob, UM (10)), qualityOutPercent (bobAliceQOut));
                env (trust (carol, UM (10)));

                env (pay (alice, bob, UM (10)));
                env.require (balance (bob, UM (10)));
                env (pay (bob, carol, UM (5)), sendmax (UM (5)));
                env.require (balance (carol, UM (5)));
                env.require (balance (bob, UM (10-5)));
            }
    }

    void testBookStep ()
    {
        testcase ("Book Step");

        using namespace jtx;

        auto const gw = Account ("gateway");
        auto const USD = gw["USD"];
        auto const BTC = gw["BTC"];
        auto const EUR = gw["EUR"];
        Account const alice ("alice");
        Account const bob ("bob");
        Account const carol ("carol");

        {
            // simple IOU/IOU offer
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);

            env (pay (gw, alice, BTC (50)));
            env (pay (gw, bob, USD (50)));

            env (offer (bob, BTC (50), USD (50)));

            env (pay (alice, carol, USD (50)), path (~USD), sendmax (BTC (50)));

            env.require (balance (alice, BTC (0)));
            env.require (balance (bob, BTC (50)));
            env.require (balance (bob, USD (0)));
            env.require (balance (carol, USD (50)));
            BEAST_EXPECT(!isOffer (env, bob, BTC (50), USD (50)));
        }
        {
            // simple IOU/M M/IOU offer
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);

            env (pay (gw, alice, BTC (50)));
            env (pay (gw, bob, USD (50)));

            env (offer (bob, BTC (50), M (50)));
            env (offer (bob, M (50), USD (50)));

            env (pay (alice, carol, USD (50)), path (~M, ~USD),
                sendmax (BTC (50)));

            env.require (balance (alice, BTC (0)));
            env.require (balance (bob, BTC (50)));
            env.require (balance (bob, USD (0)));
            env.require (balance (carol, USD (50)));
            BEAST_EXPECT(!isOffer (env, bob, M (50), USD (50)));
            BEAST_EXPECT(!isOffer (env, bob, BTC (50), M (50)));
        }
        {
            // simple M -> USD through offer and sendmax
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);

            env (pay (gw, bob, USD (50)));

            env (offer (bob, M (50), USD (50)));

            env (pay (alice, carol, USD (50)), path (~USD),
                 sendmax (M (50)));

            env.require (balance (alice, mMinusFee (env, 10000 - 50)));
            env.require (balance (bob, mMinusFee (env, 10000 + 50)));
            env.require (balance (bob, USD (0)));
            env.require (balance (carol, USD (50)));
            BEAST_EXPECT(!isOffer (env, bob, M (50), USD (50)));
        }
        {
            // simple USD -> M through offer and sendmax
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);

            env (pay (gw, alice, USD (50)));

            env (offer (bob, USD (50), M (50)));

            env (pay (alice, carol, M (50)), path (~M),
                 sendmax (USD (50)));

            env.require (balance (alice, USD (0)));
            env.require (balance (bob, mMinusFee (env, 10000 - 50)));
            env.require (balance (bob, USD (50)));
            env.require (balance (carol, M (10000 + 50)));
            BEAST_EXPECT(!isOffer (env, bob, USD (50), M (50)));
        }
        {
            // test unfunded offers are removed when payment succeeds
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);
            env.trust (EUR (1000), alice, bob, carol);

            env (pay (gw, alice, BTC (60)));
            env (pay (gw, bob, USD (50)));
            env (pay (gw, bob, EUR (50)));

            env (offer (bob, BTC (50), USD (50)));
            env (offer (bob, BTC (60), EUR (50)));
            env (offer (bob, EUR (50), USD (50)));

            // unfund offer
            env (pay (bob, gw, EUR (50)));
            BEAST_EXPECT(isOffer (env, bob, BTC (50), USD (50)));
            BEAST_EXPECT(isOffer (env, bob, BTC (60), EUR (50)));
            BEAST_EXPECT(isOffer (env, bob, EUR (50), USD (50)));

            env (pay (alice, carol, USD (50)),
                path (~USD), path (~EUR, ~USD),
                sendmax (BTC (60)));

            env.require (balance (alice, BTC (10)));
            env.require (balance (bob, BTC (50)));
            env.require (balance (bob, USD (0)));
            env.require (balance (bob, EUR (0)));
            env.require (balance (carol, USD (50)));
            // used in the payment
            BEAST_EXPECT(!isOffer (env, bob, BTC (50), USD (50)));
            // found unfunded
            BEAST_EXPECT(!isOffer (env, bob, BTC (60), EUR (50)));
            // unfunded, but should not yet be found unfunded
            BEAST_EXPECT(isOffer (env, bob, EUR (50), USD (50)));
        }
        {
            // test unfunded offers are returned when the payment fails.
            // bob makes two offers: a funded 50 USD for 50 BTC and an unfunded 50
            // EUR for 60 BTC. alice pays carol 61 USD with 61 BTC. alice only
            // has 60 BTC, so the payment will fail. The payment uses two paths:
            // one through bob's funded offer and one through his unfunded
            // offer. When the payment fails `flow` should return the unfunded
            // offer. This test is intentionally similar to the one that removes
            // unfunded offers when the payment succeeds.
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (BTC (1000), alice, bob, carol);
            env.trust (EUR (1000), alice, bob, carol);

            env (pay (gw, alice, BTC (60)));
            env (pay (gw, bob, USD (50)));
            env (pay (gw, bob, EUR (50)));

            env (offer (bob, BTC (50), USD (50)));
            env (offer (bob, BTC (60), EUR (50)));
            env (offer (bob, EUR (50), USD (50)));

            // unfund offer
            env (pay (bob, gw, EUR (50)));
            BEAST_EXPECT(isOffer (env, bob, BTC (50), USD (50)));
            BEAST_EXPECT(isOffer (env, bob, BTC (60), EUR (50)));

            auto flowJournal = env.app ().logs ().journal ("Flow");
            auto const flowResult = [&]
            {
                STAmount deliver (USD (51));
                STAmount smax (BTC (61));
                PaymentSandbox sb (env.current ().get (), tapNONE);
                STPathSet paths;
                {
                    // BTC -> USD
                    STPath p1 ({IPE (USD.issue ())});
                    paths.push_back (p1);
                    // BTC -> EUR -> USD
                    STPath p2 ({IPE (EUR.issue ()), IPE (USD.issue ())});
                    paths.push_back (p2);
                }

                return flow (sb, deliver, alice, carol, paths, false, false, true,
                    boost::none, smax, flowJournal);
            }();

            BEAST_EXPECT(flowResult.removableOffers.size () == 1);
            env.app ().openLedger ().modify (
                [&](OpenView& view, beast::Journal j)
                {
                    if (flowResult.removableOffers.empty())
                        return false;
                    Sandbox sb (&view, tapNONE);
                    for (auto const& o : flowResult.removableOffers)
                        if (auto ok = sb.peek (keylet::offer (o)))
                            offerDelete (sb, ok, flowJournal);
                    sb.apply (view);
                    return true;
                });

            // used in payment, but since payment failed should be untouched
            BEAST_EXPECT(isOffer (env, bob, BTC (50), USD (50)));
            // found unfunded
            BEAST_EXPECT(!isOffer (env, bob, BTC (60), EUR (50)));
        }
        {
            // Do not produce more in the forward pass than the reverse pass
            // This test uses a path that whose reverse pass will compute a
            // 0.5 USD input required for a 1 EUR output. It sets a sendmax of
            // 0.4 USD, so the payment engine will need to do a forward pass.
            // Without limits, the 0.4 USD would produce 1000 EUR in the forward
            // pass. This test checks that the payment produces 1 EUR, as expected.

            Env env (*this, features (featureFlow),
                features (featureOwnerPaysFee));

            auto const closeTime = STAmountSO::soTime2 +
                100 * env.closed ()->info ().closeTimeResolution;
            env.close (closeTime);

            env.fund (M (10000), alice, bob, carol, gw);
            env.trust (USD (1000), alice, bob, carol);
            env.trust (EUR (1000), alice, bob, carol);

            env (pay (gw, alice, USD (1000)));
            env (pay (gw, bob, EUR (1000)));

            env (offer (bob, USD (1), drops (2)), txflags (tfPassive));
            env (offer (bob, drops (1), EUR (1000)), txflags (tfPassive));

            env (pay (alice, carol, EUR (1)), path (~M, ~EUR),
                sendmax (USD (0.4)), txflags (tfNoMTChainDirect|tfPartialPayment));

            env.require (balance (carol, EUR (1)));
            env.require (balance (bob, USD (0.4)));
            env.require (balance (bob, EUR (999)));
        }
    }

    void testTransferRate ()
    {
        testcase ("Transfer Rate");

        using namespace jtx;

        auto const gw = Account ("gateway");
        auto const USD = gw["USD"];
        auto const BTC = gw["BTC"];
        auto const EUR = gw["EUR"];
        Account const alice ("alice");
        Account const bob ("bob");
        Account const carol ("carol");


        {
            // Simple payment through a gateway with a
            // transfer rate
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol);
            env (pay (gw, alice, USD (50)));
            env.require (balance (alice, USD (50)));
            env (pay (alice, bob, USD (40)), sendmax (USD (50)));
            env.require (balance (bob, USD (40)), balance (alice, USD (0)));
        }
        {
            // transfer rate is not charged when issuer is src or dst
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol);
            env (pay (gw, alice, USD (50)));
            env.require (balance (alice, USD (50)));
            env (pay (alice, gw, USD (40)), sendmax (USD (40)));
            env.require (balance (alice, USD (10)));
        }
        {
            // transfer fee on an offer
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol);
            env (pay (gw, bob, USD (65)));

            env (offer (bob, M (50), USD (50)));

            env (pay (alice, carol, USD (50)), path (~USD), sendmax (M (50)));
            env.require (
                balance (alice, mMinusFee (env, 10000 - 50)),
                balance (bob, USD (2.5)), // owner pays transfer fee
                balance (carol, USD (50)));
        }

        {
            // Transfer fee two consecutive offers
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, carol, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol);
            env.trust (EUR (1000), alice, bob, carol);
            env (pay (gw, bob, USD (50)));
            env (pay (gw, bob, EUR (50)));

            env (offer (bob, M (50), USD (50)));
            env (offer (bob, USD (50), EUR (50)));

            env (pay (alice, carol, EUR (40)), path (~USD, ~EUR), sendmax (M (40)));
            env.require (
                balance (alice,  mMinusFee (env, 10000 - 40)),
                balance (bob, USD (40)),
                balance (bob, EUR (0)),
                balance (carol, EUR (40)));
        }

        {
            // First pass through a strand redeems, second pass issues, no offers
            // limiting step is not an endpoint
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            auto const UM = alice["USD"];
            auto const USDB = bob["USD"];

            env.fund (M (10000), alice, bob, carol, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol);
            env.trust (UM (1000), bob);
            env.trust (USDB (1000), gw);
            env (pay (gw, bob, USD (50)));
            // alice -> bob -> gw -> carol. $50 should have transfer fee; $10, no fee
            env (pay (alice, carol, USD(50)), path (bob), sendmax (UM(60)));
            env.require (
                balance (bob, USD (-10)),
                balance (bob, UM (60)),
                balance (carol, USD (50)));
        }
        {
            // First pass through a strand redeems, second pass issues, through an offer
            // limiting step is not an endpoint
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));
            auto const UM = alice["USD"];
            auto const USDB = bob["USD"];
            Account const dan ("dan");

            env.fund (M (10000), alice, bob, carol, dan, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob, carol, dan);
            env.trust (EUR (1000), carol, dan);
            env.trust (UM (1000), bob);
            env.trust (USDB (1000), gw);
            env (pay (gw, bob, USD (50)));
            env (pay (gw, dan, EUR (100)));
            env (offer (dan, USD (100), EUR (100)));
            // alice -> bob -> gw -> carol. $50 should have transfer fee; $10, no fee
            env (pay (alice, carol, EUR (50)), path (bob, gw, ~EUR),
                sendmax (UM (60)), txflags (tfNoMTChainDirect));
            env.require (
                balance (bob, USD (-10)),
                balance (bob, UM (60)),
                balance (dan, USD (50)),
                balance (dan, EUR (37.5)),
                balance (carol, EUR (50)));
        }

        {
            // Offer where the owner is also the issuer, owner pays fee
            Env env (*this, features(featureFlow), features(featureOwnerPaysFee));

            env.fund (M (10000), alice, bob, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob);
            env (offer (gw, M (100), USD (100)));
            env (pay (alice, bob, USD (100)),
                 sendmax (M (100)));
            env.require (
                balance (alice, mMinusFee(env, 10000-100)),
                balance (bob, USD (100)));
        }
        {
            // Offer where the owner is also the issuer, sender pays fee
            Env env (*this, features(featureFlow));

            env.fund (M (10000), alice, bob, gw);
            env(rate(gw, 1.25));
            env.trust (USD (1000), alice, bob);
            env (offer (gw, M (125), USD (125)));
            env (pay (alice, bob, USD (100)),
                 sendmax (M (200)));
            env.require (
                balance (alice, mMinusFee(env, 10000-125)),
                balance (bob, USD (100)));
        }
    }

    void
    testFalseDryHelper (jtx::Env& env)
    {
        using namespace jtx;

        auto const gw = Account ("gateway");
        auto const USD = gw["USD"];
        auto const EUR = gw["EUR"];
        Account const alice ("alice");
        Account const bob ("bob");
        Account const carol ("carol");

        auto const closeTime = amendmentRIPD1141SoTime() +
                100 * env.closed ()->info ().closeTimeResolution;
        env.close (closeTime);

        env.fund (M(10000), alice, carol, gw);
        env.fund (reserve(env, 5), bob);
        env.trust (USD (1000), alice, bob, carol);
        env.trust (EUR (1000), alice, bob, carol);


        env (pay (gw, alice, EUR (50)));
        env (pay (gw, bob, USD (50)));

        // Bob has _just_ slightly less than 50 m available
        // If his owner count changes, he will have more liquidity.
        // This is one error case to test (when Flow is used).
        // Computing the incomming m to the M/USD offer will require two
        // recursive calls to the EUR/M offer. The second call will return
        // tecPATH_DRY, but the entire path should not be marked as dry. This
        // is the second error case to test (when flowV1 is used).
        env (offer (bob, EUR (50), M (50)));
        env (offer (bob, M (50), USD (50)));

        env (pay (alice, carol, USD (1000000)), path (~M, ~USD),
            sendmax (EUR (500)),
            txflags (tfNoMTChainDirect | tfPartialPayment));

        auto const carolUSD = env.balance(carol, USD).value();
        BEAST_EXPECT(carolUSD > USD (0) && carolUSD < USD (50));
    }

    void
    testFalseDry ()
    {
        testcase ("falseDryChanges");
        using namespace jtx;
        {
            Env env (*this, features (featureFlow));
            testFalseDryHelper (env);
        }
        {
            Env env (*this);
            testFalseDryHelper (env);
        }
    }

    void
    testLimitQuality ()
    {
        // Single path with two offers and limit quality. The quality limit is
        // such that the first offer should be taken but the second should not.
        // The total amount delivered should be the sum of the two offers and
        // sendMax should be more than the first offer.
        testcase ("limitQuality");
        using namespace jtx;

        auto const gw = Account ("gateway");
        auto const USD = gw["USD"];
        Account const alice ("alice");
        Account const bob ("bob");
        Account const carol ("carol");

        auto const timeDelta = Env{*this}.closed ()->info ().closeTimeResolution;

        for(auto const& d: {-timeDelta*100, +timeDelta*100}){
            auto const closeTime = amendmentRIPD1141SoTime () + d;
            Env env (*this);
            env.close (closeTime);

            env.fund (M(10000), alice, bob, carol, gw);

            env.trust (USD(100), alice, bob, carol);
            env (pay (gw, bob, USD (100)));
            env (offer (bob, M (50), USD (50)));
            env (offer (bob, M (100), USD (50)));

            auto expectedResult =
                closeTime < amendmentRIPD1141SoTime () ? tecPATH_DRY : tesSUCCESS;
            env (pay (alice, carol, USD (100)), path (~USD), sendmax (M (100)),
                txflags (tfNoMTChainDirect | tfPartialPayment | tfLimitQuality),
                ter (expectedResult));

            if (expectedResult == tesSUCCESS)
                env.require (balance (carol, USD (50)));
        }
    }

    // Helper function that returns the reserve on an account based on
    // the passed in number of owners.
    static MAmount reserve(jtx::Env& env, std::uint32_t count)
    {
        return env.current()->fees().accountReserve (count);
    }

    // Helper function that returns the Offers on an account.
    static std::vector<std::shared_ptr<SLE const>>
    offersOnAccount (jtx::Env& env, jtx::Account account)
    {
        std::vector<std::shared_ptr<SLE const>> result;
        forEachItem (*env.current (), account,
            [&env, &result](std::shared_ptr<SLE const> const& sle)
            {
                if (sle->getType() == ltOFFER)
                     result.push_back (sle);
            });
        return result;
    }

    void
    testSelfPayment1()
    {
        testcase ("Self-payment 1");

        // In this test case the new flow code mis-computes the amount
        // of money to move.  Fortunately the new code's re-execute
        // check catches the problem and throws out the transaction.
        //
        // The old payment code handles the payment correctly.
        using namespace jtx;

        auto const gw1 = Account ("gw1");
        auto const gw2 = Account ("gw2");
        auto const alice = Account ("alice");
        auto const USD = gw1["USD"];
        auto const EUR = gw2["EUR"];

        Env env (*this, features (featureFlow));

        auto const closeTime =
            amendmentRIPD1141SoTime () + 100 * env.closed ()->info ().closeTimeResolution;
        env.close (closeTime);

        env.fund (M (1000000), gw1, gw2);
        env.close ();

        // The fee that's charged for transactions.
        auto const f = env.current ()->fees ().base;

        env.fund (reserve (env, 3) + f * 4, alice);
        env.close ();

        env (trust (alice, USD (2000)));
        env (trust (alice, EUR (2000)));
        env.close ();

        env (pay (gw1, alice, USD (1)));
        env (pay (gw2, alice, EUR (1000)));
        env.close ();

        env (offer (alice, USD (500), EUR (600)));
        env.close ();

        env.require (owners (alice, 3));
        env.require (balance (alice, USD (1)));
        env.require (balance (alice, EUR (1000)));

        auto aliceOffers = offersOnAccount (env, alice);
        BEAST_EXPECT(aliceOffers.size () == 1);
        for (auto const& offerPtr : aliceOffers)
        {
            auto const offer = *offerPtr;
            BEAST_EXPECT(offer[sfLedgerEntryType] == ltOFFER);
            BEAST_EXPECT(offer[sfTakerGets] == EUR (600));
            BEAST_EXPECT(offer[sfTakerPays] == USD (500));
        }

        env (pay (alice, alice, EUR (600)), sendmax (USD (500)),
            txflags (tfPartialPayment));
        env.close ();

        env.require (owners (alice, 3));
        env.require (balance (alice, USD (1)));
        env.require (balance (alice, EUR (1000)));
        aliceOffers = offersOnAccount (env, alice);
        BEAST_EXPECT(aliceOffers.size () == 1);
        for (auto const& offerPtr : aliceOffers)
        {
            auto const offer = *offerPtr;
            BEAST_EXPECT(offer[sfLedgerEntryType] == ltOFFER);
            BEAST_EXPECT(offer[sfTakerGets] == EUR (598.8));
            BEAST_EXPECT(offer[sfTakerPays] == USD (499));
        }
    }

    void
    testSelfPayment2()
    {
        testcase ("Self-payment 2");

        // In this case the difference between the old payment code and
        // the new is the values left behind in the offer.  Not saying either
        // ios ring, they are just different.
        using namespace jtx;

        auto const gw1 = Account ("gw1");
        auto const gw2 = Account ("gw2");
        auto const alice = Account ("alice");
        auto const USD = gw1["USD"];
        auto const EUR = gw2["EUR"];

        Env env (*this, features (featureFlow));

        auto const closeTime =
            amendmentRIPD1141SoTime () + 100 * env.closed ()->info ().closeTimeResolution;
        env.close (closeTime);

        env.fund (M (1000000), gw1, gw2);
        env.close ();

        // The fee that's charged for transactions.
        auto const f = env.current ()->fees ().base;

        env.fund (reserve (env, 3) + f * 4, alice);
        env.close ();

        env (trust (alice, USD (506)));
        env (trust (alice, EUR (606)));
        env.close ();

        env (pay (gw1, alice, USD (500)));
        env (pay (gw2, alice, EUR (600)));
        env.close ();

        env (offer (alice, USD (500), EUR (600)));
        env.close ();

        env.require (owners (alice, 3));
        env.require (balance (alice, USD (500)));
        env.require (balance (alice, EUR (600)));

        auto aliceOffers = offersOnAccount (env, alice);
        BEAST_EXPECT(aliceOffers.size () == 1);
        for (auto const& offerPtr : aliceOffers)
        {
            auto const offer = *offerPtr;
            BEAST_EXPECT(offer[sfLedgerEntryType] == ltOFFER);
            BEAST_EXPECT(offer[sfTakerGets] == EUR (600));
            BEAST_EXPECT(offer[sfTakerPays] == USD (500));
        }

        env (pay (alice, alice, EUR (60)), sendmax (USD (50)),
            txflags (tfPartialPayment));
        env.close ();

        env.require (owners (alice, 3));
        env.require (balance (alice, USD (500)));
        env.require (balance (alice, EUR (600)));
        aliceOffers = offersOnAccount (env, alice);
        BEAST_EXPECT(aliceOffers.size () == 1);
        for (auto const& offerPtr : aliceOffers)
        {
            auto const offer = *offerPtr;
            BEAST_EXPECT(offer[sfLedgerEntryType] == ltOFFER);
            BEAST_EXPECT(offer[sfTakerGets] == EUR (594));
            BEAST_EXPECT(offer[sfTakerPays] == USD (495));
        }
    }
    void testSelfFundedMEndpoint (bool consumeOffer)
    {
        // Test that the deferred credit table is not bypassed for
        // MEndpointSteps. If the account in the first step is sending M and
        // that account also owns an offer that receives M, it should not be
        // possible for that step to use the M received in the offer as part
        // of the payment.
        testcase("Self funded MEndpoint");

        using namespace jtx;

        Env env(*this, features(featureFlow));

        // Need new behavior from `accountHolds`
        auto const closeTime = amendmentRIPD1141SoTime() +
            env.closed()->info().closeTimeResolution;
        env.close(closeTime);

        auto const alice = Account("alice");
        auto const gw = Account("gw");
        auto const USD = gw["USD"];

        env.fund(M(10000), alice, gw);
        env(trust(alice, USD(20)));
        env(pay(gw, alice, USD(10)));
        env(offer(alice, M(50000), USD(10)));

        // Consuming the offer changes the owner count, which could also cause
        // liquidity to decrease in the forward pass
        auto const toSend = consumeOffer ? USD(10) : USD(9);
        env(pay(alice, alice, toSend), path(~USD), sendmax(M(20000)),
            txflags(tfPartialPayment | tfNoMTChainDirect));
    }

    void testUnfundedOffer (bool withFix)
    {
        testcase(std::string("Unfunded Offer ") +
            (withFix ? "with fix" : "without fix"));

        using namespace jtx;
        {
            // Test reverse
            Env env(*this, features(featureFlow));
            auto closeTime = amendmentRIPD1298SoTime();
            if (withFix)
                closeTime += env.closed()->info().closeTimeResolution;
            else
                closeTime -= env.closed()->info().closeTimeResolution;
            env.close(closeTime);

            auto const alice = Account("alice");
            auto const bob = Account("bob");
            auto const gw = Account("gw");
            auto const USD = gw["USD"];

            env.fund(M(100000), alice, bob, gw);
            env(trust(bob, USD(20)));

            STAmount tinyAmt1{USD.issue(), 9000000000000000ll, -17, false,
                false, STAmount::unchecked{}};
            STAmount tinyAmt3{USD.issue(), 9000000000000003ll, -17, false,
                false, STAmount::unchecked{}};

            env(offer(gw, drops(9000000000), tinyAmt3));
            env(pay(alice, bob, tinyAmt1), path(~USD),
                sendmax(drops(9000000000)), txflags(tfNoMTChainDirect));

            if (withFix)
                BEAST_EXPECT(!isOffer(env, gw, M(0), USD(0)));
            else
                BEAST_EXPECT(isOffer(env, gw, M(0), USD(0)));
        }
        {
            // Test forward
            Env env(*this, features(featureFlow));
            auto closeTime = amendmentRIPD1298SoTime();
            if (withFix)
                closeTime += env.closed()->info().closeTimeResolution;
            else
                closeTime -= env.closed()->info().closeTimeResolution;
            env.close(closeTime);

            auto const alice = Account("alice");
            auto const bob = Account("bob");
            auto const gw = Account("gw");
            auto const USD = gw["USD"];

            env.fund(M(100000), alice, bob, gw);
            env(trust(alice, USD(20)));

            STAmount tinyAmt1{USD.issue(), 9000000000000000ll, -17, false,
                false, STAmount::unchecked{}};
            STAmount tinyAmt3{USD.issue(), 9000000000000003ll, -17, false,
                false, STAmount::unchecked{}};

            env(pay(gw, alice, tinyAmt1));

            env(offer(gw, tinyAmt3, drops(9000000000)));
            env(pay(alice, bob, drops(9000000000)), path(~M),
                sendmax(USD(1)), txflags(tfNoMTChainDirect));

            if (withFix)
                BEAST_EXPECT(!isOffer(env, gw, USD(0), M(0)));
            else
                BEAST_EXPECT(isOffer(env, gw, USD(0), M(0)));
        }
    }

    template<class... Features>
    void
    testReexecuteDirectStep(Features&&... fs)
    {
        testcase("ReexecuteDirectStep");

        using namespace jtx;
        Env env(*this, features(fs)...);

        auto const alice = Account("alice");
        auto const bob = Account("bob");
        auto const gw = Account("gw");
        auto const USD = gw["USD"];

        env.fund(M(10000), alice, bob, gw);
        env(trust(alice, USD(100)));

        env(pay(
            gw, alice,
            // 12.55....
            STAmount{USD.issue(), std::uint64_t(1255555555555555ull), -14, false}));

        env(offer(gw,
            // 5.0...
            STAmount{
                USD.issue(), std::uint64_t(5000000000000000ull), -15, false},
            M(1000)));

        env(offer(gw,
            // .555...
            STAmount{
                USD.issue(), std::uint64_t(5555555555555555ull), -16, false},
            M(10)));

        env(offer(gw,
            // 4.44....
            STAmount{
                USD.issue(), std::uint64_t(4444444444444444ull), -15, false},
            M(.1)));

        env(offer(alice,
            // 17
            STAmount{
                USD.issue(), std::uint64_t(1700000000000000ull), -14, false},
            M(.001)));

        // Need to be past this time to see the bug
        env.close(amendmentRIPD1274SoTime() +
            100 * env.closed()->info().closeTimeResolution);

        env(pay(alice, bob, M(10000)), path(~M), sendmax(USD(100)),
            txflags(tfPartialPayment | tfNoMTChainDirect));
    }

    void
    testRIPD1443(bool withFix)
    {
        testcase("ripd1443");

        using namespace jtx;
        Env env(*this, features(featureFlow));
        auto const timeDelta = env.closed ()->info ().closeTimeResolution;
        auto const d = withFix ? timeDelta*100 : -timeDelta*100;
        auto closeTime = amendmentRIPD1443SoTime() + d;
        env.close(closeTime);

        auto const alice = Account("alice");
        auto const bob = Account("bob");
        auto const carol = Account("carol");
        auto const gw = Account("gw");

        env.fund(M(100000000), alice, nomtchain(bob), carol, gw);
        env.trust(gw["USD"](10000), alice, carol);
        env(trust(bob, gw["USD"](10000), tfSetNoMtchain));
        env.trust(gw["USD"](10000), bob);
        env.close();

        // set no mtchain between bob and the gateway

        env(pay(gw, alice, gw["USD"](1000)));
        env.close();

        env(offer(alice, bob["USD"](1000), M(1)));
        env.close();

        env(pay(alice, alice, M(1)), path(gw, bob, ~M),
            sendmax(gw["USD"](1000)), txflags(tfNoMTChainDirect),
            ter(withFix ? tecPATH_DRY : tesSUCCESS));
        env.close();

        if (withFix)
        {
            env.trust(bob["USD"](10000), alice);
            env(pay(bob, alice, bob["USD"](1000)));
        }

        env(offer(alice, M(1000), bob["USD"](1000)));
        env.close();

        env(pay (carol, carol, gw["USD"](1000)), path(~bob["USD"], gw),
            sendmax(M(100000)), txflags(tfNoMTChainDirect),
            ter(withFix ? tecPATH_DRY : tesSUCCESS));
        env.close();

        pass();
    }

    void run() override
    {
        testDirectStep ();
        testLineQuality();
        testBookStep ();
        testTransferRate ();
        testToStrand ();
        testFalseDry();
        testLimitQuality();
        testSelfPayment1();
        testSelfPayment2();
        testSelfFundedMEndpoint(false);
        testSelfFundedMEndpoint(true);
        testUnfundedOffer(true);
        testUnfundedOffer(false);
        testReexecuteDirectStep(featureFlow, fix1368);
        testRIPD1443(true);
        testRIPD1443(false);
    }
};

BEAST_DEFINE_TESTSUITE(Flow,app,mtchain);

} // test
} //
