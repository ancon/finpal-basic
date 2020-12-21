//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/Credit.h>
#include <mtchain/app/paths/Flow.h>
#include <mtchain/app/paths/impl/AmountSpec.h>
#include <mtchain/app/paths/impl/StrandFlow.h>
#include <mtchain/app/paths/impl/Steps.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/IOUAmount.h>
#include <mtchain/protocol/MAmount.h>

#include <boost/container/flat_set.hpp>

#include <numeric>
#include <sstream>

namespace mtchain {

template<class FlowResult>
static
auto finishFlow (PaymentSandbox& sb,
    Issue const& srcIssue, Issue const& dstIssue,
    FlowResult&& f)
{
    path::MtchainCalc::Output result;
    if (f.ter == tesSUCCESS)
        f.sandbox->apply (sb);
    else
        result.removableOffers = std::move (f.removableOffers);

    result.setResult (f.ter);
    result.actualAmountIn = toSTAmount (f.in, srcIssue);
    result.actualAmountOut = toSTAmount (f.out, dstIssue);

    return result;
};

path::MtchainCalc::Output
flow (
    PaymentSandbox& sb,
    STAmount const& deliver,
    AccountID const& src,
    AccountID const& dst,
    STPathSet const& paths,
    bool defaultPaths,
    bool partialPayment,
    bool ownerPaysTransferFee,
    boost::optional<Quality> const& limitQuality,
    boost::optional<STAmount> const& sendMax,
    beast::Journal j,
    path::detail::FlowDebugInfo* flowDebugInfo)
{
    Issue const srcIssue = [&] {
        if (sendMax)
            return sendMax->issue ();
        if (!isM (deliver.issue ().currency))
            return Issue (deliver.issue ().currency, src);
        return mIssue ();
    }();

    Issue const dstIssue = deliver.issue ();

    boost::optional<Issue> sendMaxIssue;
    if (sendMax)
        sendMaxIssue = sendMax->issue ();

    // convert the paths to a collection of strands. Each strand is the collection
    // of account->account steps and book steps that may be used in this payment.
    auto sr = toStrands (sb, src, dst, dstIssue, sendMaxIssue, paths,
        defaultPaths, ownerPaysTransferFee, j);

    if (sr.first != tesSUCCESS)
    {
        path::MtchainCalc::Output result;
        result.setResult (sr.first);
        return result;
    }

    auto& strands = sr.second;

    if (j.trace())
    {
        j.trace() << "\nsrc: " << src << "\ndst: " << dst
            << "\nsrcIssue: " << srcIssue << "\ndstIssue: " << dstIssue;
        j.trace() << "\nNumStrands: " << strands.size ();
        for (auto const& curStrand : strands)
        {
            j.trace() << "NumSteps: " << curStrand.size ();
            for (auto const& step : curStrand)
            {
                j.trace() << '\n' << *step << '\n';
            }
        }
    }

    const bool srcIsM = isM (srcIssue.currency);
    const bool dstIsM = isM (dstIssue.currency);

    auto const asDeliver = toAmountSpec (deliver);

    // The src account may send either m or iou. The dst account may receive
    // either m or iou. Since M and IOU amounts are represented by different
    // types, use templates to tell `flow` about the amount types.
    if (srcIsM && dstIsM)
    {
        return finishFlow (sb, srcIssue, dstIssue,
            flow<MAmount, MAmount> (
                sb, strands, asDeliver.m, defaultPaths, partialPayment,
                limitQuality, sendMax, j, flowDebugInfo));
    }

    if (srcIsM && !dstIsM)
    {
        return finishFlow (sb, srcIssue, dstIssue,
            flow<MAmount, IOUAmount> (
                sb, strands, asDeliver.iou, defaultPaths, partialPayment,
                limitQuality, sendMax, j, flowDebugInfo));
    }

    if (!srcIsM && dstIsM)
    {
        return finishFlow (sb, srcIssue, dstIssue,
            flow<IOUAmount, MAmount> (
                sb, strands, asDeliver.m, defaultPaths, partialPayment,
                limitQuality, sendMax, j, flowDebugInfo));
    }

    assert (!srcIsM && !dstIsM);
    auto rc = finishFlow (sb, srcIssue, dstIssue, flow<IOUAmount, IOUAmount> (
                          sb, strands, asDeliver.iou, defaultPaths, partialPayment,
                          limitQuality, sendMax, j, flowDebugInfo));


    //在发行商的两个客户之间转账
    if (rc.result() == tesSUCCESS && paths.empty() && src != dstIssue.account && dst != dstIssue.account && dstIssue.currency == srcIssue.currency)
    {
        auto const sle = sb.peek(keylet::account(dstIssue.account));
        if (sle->isFieldPresent(sfTransferFee))
        {
            auto const& tf = sle->getFieldAmount(sfTransferFee);
            //if (tf.issue() != dstIssue)
            {
                sr = toStrands(sb, src, dstIssue.account, tf.issue(), boost::none, STPathSet(),
                               true, false, j);
                if (sr.first != tesSUCCESS)
                {
                    rc.setResult(sr.first == terNO_LINE ? tecNO_LINE_IOU_FEE : tecFAILED_IOU_FEE);
                }
                else
                {
                    auto result = flow<IOUAmount, IOUAmount> (sb, sr.second, toAmountSpec(tf).iou,
                                         true, false, boost::none, boost::none, j, flowDebugInfo);
                    if (result.ter != tesSUCCESS)
                    {
                        rc.setResult(result.ter == tecUNFUNDED_PAYMENT_IOU ?
                                     tecUNFUNDED_PAYMENT_IOU_FEE : tecFAILED_IOU_FEE);
		    }
                    else
                    {
                        rc.fee = tf;
                        finishFlow(sb, dstIssue, dstIssue, result);
                    }
                }
            }
        }
    }

    return rc;
}

} //
