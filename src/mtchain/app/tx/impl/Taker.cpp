//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/Taker.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>

namespace mtchain {

static
std::string
format_amount (STAmount const& amount)
{
    std::string txt = amount.getText ();
    txt += "/";
    txt += to_string (amount.issue().currency);
    return txt;
}

static
Rate
calculateRate (
    ApplyView const& view,
    Issue const& issue,
    AccountID const& account)
{
    auto const& issuer = issue.account;
    return isM (issuer) || (account == issuer)
        ? parityRate
        : transferRate (view, issuer, issue.currency);
}

BasicTaker::BasicTaker (
        CrossType cross_type, AccountID const& account, Amounts const& amount,
        Quality const& quality, std::uint32_t flags, Rate const& rate_in,
        Rate const& rate_out, beast::Journal journal)
    : account_ (account)
    , quality_ (quality)
    , threshold_ (quality_)
    , sell_ (flags & tfSell)
    , original_ (amount)
    , remaining_ (amount)
    , issue_in_ (remaining_.in.issue ())
    , issue_out_ (remaining_.out.issue ())
    , m_rate_in (rate_in)
    , m_rate_out (rate_out)
    , cross_type_ (cross_type)
    , journal_ (journal)
{
    assert (remaining_.in > zero);
    assert (remaining_.out > zero);

    assert (m_rate_in.value != 0);
    assert (m_rate_out.value != 0);

    // If we are dealing with a particular flavor, make sure that it's the
    // flavor we expect:
    assert (cross_type != CrossType::MToIou ||
        (isM (issue_in ()) && !isM (issue_out ())));

    assert (cross_type != CrossType::IouToM ||
        (!isM (issue_in ()) && isM (issue_out ())));

    // And make sure we're not crossing M for M
    assert (!isM (issue_in ()) || !isM (issue_out ()));

    // If this is a passive order, we adjust the quality so as to prevent offers
    // at the same quality level from being consumed.
    if (flags & tfPassive)
        ++threshold_;
}

Rate
BasicTaker::effective_rate (
    Rate const& rate, Issue const &issue,
    AccountID const& from, AccountID const& to)
{
    // If there's a transfer rate, the issuer is not involved
    // and the sender isn't the same as the recipient, return
    // the actual transfer rate.
    if (rate != parityRate &&
        from != to &&
        from != issue.account &&
        to != issue.account)
    {
        return rate;
    }

    return parityRate;
}

bool
BasicTaker::unfunded () const
{
    if (get_funds (account(), remaining_.in) > zero)
        return false;

    JLOG(journal_.debug()) << "Unfunded: taker is out of funds.";
    return true;
}

bool
BasicTaker::done () const
{
    // We are done if we have consumed all the input currency
    if (remaining_.in <= zero)
    {
        JLOG(journal_.debug()) << "Done: all the input currency has been consumed.";
        return true;
    }

    // We are done if using buy semantics and we received the
    // desired amount of output currency
    if (!sell_ && (remaining_.out <= zero))
    {
        JLOG(journal_.debug()) << "Done: the desired amount has been received.";
        return true;
    }

    // We are done if the taker is out of funds
    if (unfunded ())
    {
        JLOG(journal_.debug()) << "Done: taker out of funds.";
        return true;
    }

    return false;
}

Amounts
BasicTaker::remaining_offer () const
{
    // If the taker is done, then there's no offer to place.
    if (done ())
        return Amounts (remaining_.in.zeroed(), remaining_.out.zeroed());

    // Avoid math altogether if we didn't cross.
    if (original_ == remaining_)
       return original_;

    if (sell_)
    {
        assert (remaining_.in > zero);

        // We scale the output based on the remaining input:
        return Amounts (remaining_.in, divRound (
            remaining_.in, quality_.rate (), issue_out_, true));
    }

    assert (remaining_.out > zero);

    // We scale the input based on the remaining output:
    return Amounts (mulRound (
        remaining_.out, quality_.rate (), issue_in_, true),
        remaining_.out);
}

Amounts const&
BasicTaker::original_offer () const
{
    return original_;
}

// TODO: the presence of 'output' is an artifact caused by the fact that
// Amounts carry issue information which should be decoupled.
static
STAmount
qual_div (STAmount const& amount, Quality const& quality, STAmount const& output)
{
    auto result = divide (amount, quality.rate (), output.issue ());
    return std::min (result, output);
}

static
STAmount
qual_mul (STAmount const& amount, Quality const& quality, STAmount const& output)
{
    auto result = multiply (amount, quality.rate (), output.issue ());
    return std::min (result, output);
}

void
BasicTaker::log_flow (char const* description, Flow const& flow)
{
    auto stream = journal_.debug();
    if (!stream)
        return;

    stream << description;

    if (isM (issue_in ()))
        stream << "   order in: " << format_amount (flow.order.in);
    else
        stream << "   order in: " << format_amount (flow.order.in) <<
            " (issuer: " << format_amount (flow.issuers.in) << ")";

    if (isM (issue_out ()))
        stream << "  order out: " << format_amount (flow.order.out);
    else
        stream << "  order out: " << format_amount (flow.order.out) <<
            " (issuer: " << format_amount (flow.issuers.out) << ")";
}

BasicTaker::Flow
BasicTaker::flow_m_to_iou (
    Amounts const& order, Quality quality,
    STAmount const& owner_funds, STAmount const& taker_funds,
    Rate const& rate_out)
{
    Flow f;
    f.order = order;
    f.issuers.out = multiply (f.order.out, rate_out);

    log_flow ("flow_m_to_iou", f);

    // Clamp on owner balance
    if (owner_funds < f.issuers.out)
    {
        f.issuers.out = owner_funds;
        f.order.out = divide (f.issuers.out, rate_out);
        f.order.in = qual_mul (f.order.out, quality, f.order.in);
        log_flow ("(clamped on owner balance)", f);
    }

    // Clamp if taker wants to limit the output
    if (!sell_ && remaining_.out < f.order.out)
    {
        f.order.out = remaining_.out;
        f.order.in = qual_mul (f.order.out, quality, f.order.in);
        f.issuers.out = multiply (f.order.out, rate_out);
        log_flow ("(clamped on taker output)", f);
    }

    // Clamp on the taker's funds
    if (taker_funds < f.order.in)
    {
        f.order.in = taker_funds;
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        f.issuers.out = multiply (f.order.out, rate_out);
        log_flow ("(clamped on taker funds)", f);
    }

    // Clamp on remaining offer if we are not handling the second leg
    // of an autobridge.
    if (cross_type_ == CrossType::MToIou && (remaining_.in < f.order.in))
    {
        f.order.in = remaining_.in;
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        f.issuers.out = multiply (f.order.out, rate_out);
        log_flow ("(clamped on taker input)", f);
    }

    return f;
}

BasicTaker::Flow
BasicTaker::flow_iou_to_m (
    Amounts const& order, Quality quality,
    STAmount const& owner_funds, STAmount const& taker_funds,
    Rate const& rate_in)
{
    Flow f;
    f.order = order;
    f.issuers.in = multiply (f.order.in, rate_in);

    log_flow ("flow_iou_to_m", f);

    // Clamp on owner's funds
    if (owner_funds < f.order.out)
    {
        f.order.out = owner_funds;
        f.order.in = qual_mul (f.order.out, quality, f.order.in);
        f.issuers.in = multiply (f.order.in, rate_in);
        log_flow ("(clamped on owner funds)", f);
    }

    // Clamp if taker wants to limit the output and we are not the
    // first leg of an autobridge.
    if (!sell_ && cross_type_ == CrossType::IouToM)
    {
        if (remaining_.out < f.order.out)
        {
            f.order.out = remaining_.out;
            f.order.in = qual_mul (f.order.out, quality, f.order.in);
            f.issuers.in = multiply (f.order.in, rate_in);
            log_flow ("(clamped on taker output)", f);
        }
    }

    // Clamp on the taker's input offer
    if (remaining_.in < f.order.in)
    {
        f.order.in = remaining_.in;
        f.issuers.in = multiply (f.order.in, rate_in);
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        log_flow ("(clamped on taker input)", f);
    }

    // Clamp on the taker's input balance
    if (taker_funds < f.issuers.in)
    {
        f.issuers.in = taker_funds;
        f.order.in = divide (f.issuers.in, rate_in);
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        log_flow ("(clamped on taker funds)", f);
    }

    return f;
}

BasicTaker::Flow
BasicTaker::flow_iou_to_iou (
    Amounts const& order, Quality quality,
    STAmount const& owner_funds, STAmount const& taker_funds,
    Rate const& rate_in, Rate const& rate_out)
{
    Flow f;
    f.order = order;
    f.issuers.in = multiply (f.order.in, rate_in);
    f.issuers.out = multiply (f.order.out, rate_out);

    log_flow ("flow_iou_to_iou", f);

    // Clamp on owner balance
    if (owner_funds < f.issuers.out)
    {
        f.issuers.out = owner_funds;
        f.order.out = divide (f.issuers.out, rate_out);
        f.order.in = qual_mul (f.order.out, quality, f.order.in);
        f.issuers.in = multiply (f.order.in, rate_in);
        log_flow ("(clamped on owner funds)", f);
    }

    // Clamp on taker's offer
    if (!sell_ && remaining_.out < f.order.out)
    {
        f.order.out = remaining_.out;
        f.order.in = qual_mul (f.order.out, quality, f.order.in);
        f.issuers.out = multiply (f.order.out, rate_out);
        f.issuers.in = multiply (f.order.in, rate_in);
        log_flow ("(clamped on taker output)", f);
    }

    // Clamp on the taker's input offer
    if (remaining_.in < f.order.in)
    {
        f.order.in = remaining_.in;
        f.issuers.in = multiply (f.order.in, rate_in);
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        f.issuers.out = multiply (f.order.out, rate_out);
        log_flow ("(clamped on taker input)", f);
    }

    // Clamp on the taker's input balance
    if (taker_funds < f.issuers.in)
    {
        f.issuers.in = taker_funds;
        f.order.in = divide (f.issuers.in, rate_in);
        f.order.out = qual_div (f.order.in, quality, f.order.out);
        f.issuers.out = multiply (f.order.out, rate_out);
        log_flow ("(clamped on taker funds)", f);
    }

    return f;
}

// Calculates the direct flow through the specified offer
BasicTaker::Flow
BasicTaker::do_cross (Offer & offer)
{
    auto const& amounts = offer.amount ();
    auto const& owner = offer.owner ();
    auto const& quality = offer.quality ();
    auto const owner_funds = get_funds (owner, amounts.out);
    auto const taker_funds = get_funds (account (), amounts.in);

    Flow result;

    if (cross_type_ == CrossType::MToIou)
    {
        result = flow_m_to_iou (amounts, quality, owner_funds, taker_funds,
            out_rate (owner, account ()));
    }
    else if (cross_type_ == CrossType::IouToM)
    {
        result = flow_iou_to_m (amounts, quality, owner_funds, taker_funds,
            in_rate (owner, account ()));
    }
    else
    {
        result = flow_iou_to_iou (amounts, quality, owner_funds, taker_funds,
            in_rate (owner, account ()), out_rate (owner, account ()));
    }

    if (!result.sanity_check ())
        Throw<std::logic_error> ("Computed flow fails sanity check.");

    if (cross_type_ != CrossType::IouToM && offer.getTotalFee().signum() > 0)
    {
        auto fee = calcTxnFee (owner, account(), result.order.out, result.order.in,
                               offer.original_amount().out, offer.original_amount().in,
                               offer.getTotalFee(), offer.isSeller());

        if (fee.signum() > 0)
        {
            auto max_fee = get_funds (owner, fee);
            if (max_fee < fee)
            {
                offer.setComplete();
                updateFlowByScale (result, max_fee, fee);
            }
        }
    }

    remaining_.out -= result.order.out;
    remaining_.in -= result.order.in;

    assert (remaining_.in >= zero);

    return result;
}

// Calculates the bridged flow through the specified offers
std::pair<BasicTaker::Flow, BasicTaker::Flow>
BasicTaker::do_cross (Offer &offer1, Offer &offer2)
{
    auto const& amounts1 = offer1.amount ();
    auto const& amounts2 = offer2.amount ();

    assert (!amounts1.in.native ());
    assert (amounts1.out.native ());
    assert (amounts2.in.native ());
    assert (!amounts2.out.native ());

    auto const& owner1 = offer1.owner ();
    auto const& owner2 = offer2.owner ();

    // If the taker owns the first leg of the offer, then the taker's available
    // funds aren't the limiting factor for the input - the offer itself is.
    auto leg1_in_funds = get_funds (account (), amounts1.in);

    if (account () == owner1)
    {
        JLOG(journal_.trace()) << "The taker owns the first leg of a bridge.";
        leg1_in_funds = std::max (leg1_in_funds, amounts1.in);
    }

    // If the taker owns the second leg of the offer, then the taker's available
    // funds are not the limiting factor for the output - the offer itself is.
    auto leg2_out_funds = get_funds (owner2, amounts2.out);

    if (account () == owner2)
    {
        JLOG(journal_.trace()) << "The taker owns the second leg of a bridge.";
        leg2_out_funds = std::max (leg2_out_funds, amounts2.out);
    }

    // The amount available to flow via M is the amount that the owner of the
    // first leg of the bridge has, up to the first leg's output.
    //
    // But, when both legs of a bridge are owned by the same person, the amount
    // of M that can flow between the two legs is, essentially, infinite
    // since all the owner is doing is taking out M of his left pocket
    // and putting it in his right pocket. In that case, we set the available
    // M to the largest of the two offers.
    auto m_funds = get_funds (owner1, amounts1.out);

    if (owner1 == owner2)
    {
        JLOG(journal_.trace()) <<
            "The bridge endpoints are owned by the same account.";
        m_funds = std::max (amounts1.out, amounts2.in);
    }

    if (auto stream = journal_.debug())
    {
        stream << "Available bridge funds:";
        stream << "  leg1 in: " << format_amount (leg1_in_funds);
        stream << " leg2 out: " << format_amount (leg2_out_funds);
        stream << "      m: " << format_amount (m_funds);
    }

    auto const leg1_rate = in_rate (owner1, account ());
    auto const leg2_rate = out_rate (owner2, account ());
    auto const& quality1 = offer1.quality ();
    auto const& quality2 = offer2.quality ();

    // Attempt to determine the maximal flow that can be achieved across each
    // leg independent of the other.
    auto flow1 = flow_iou_to_m (amounts1, quality1, m_funds, leg1_in_funds, leg1_rate);

    if (!flow1.sanity_check ())
        Throw<std::logic_error> ("Computed flow1 fails sanity check.");

    auto flow2 = flow_m_to_iou (amounts2, quality2, leg2_out_funds, m_funds, leg2_rate);

    if (!flow2.sanity_check ())
        Throw<std::logic_error> ("Computed flow2 fails sanity check.");

    // We now have the maximal flows across each leg individually. We need to
    // equalize them, so that the amount of M that flows out of the first leg
    // is the same as the amount of M that flows into the second leg. We take
    // the side which is the limiting factor (if any) and adjust the other.
    if (flow1.order.out < flow2.order.in)
    {
        // Adjust the second leg of the offer down:
        flow2.order.in = flow1.order.out;
        flow2.order.out = qual_div (flow2.order.in, quality2, flow2.order.out);
        flow2.issuers.out = multiply (flow2.order.out, leg2_rate);
        log_flow ("Balancing: adjusted second leg down", flow2);
    }
    else if (flow1.order.out > flow2.order.in)
    {
        // Adjust the first leg of the offer down:
        flow1.order.out = flow2.order.in;
        flow1.order.in = qual_mul (flow1.order.out, quality1, flow1.order.in);
        flow1.issuers.in = multiply (flow1.order.in, leg1_rate);
        log_flow ("Balancing: adjusted first leg down", flow2);
    }

    if (owner2 != account() && offer2.getTotalFee().signum() > 0)
    {
        auto fee = calcTxnFee (owner2, account(), flow2.order.out, flow2.order.in,
                               offer2.original_amount().out, offer2.original_amount().in,
                               offer2.getTotalFee(), offer2.isSeller());
        if (fee.signum() > 0)
        {
            auto max_fee = get_funds (owner2, fee);
            if (max_fee < fee)
            {
                offer2.setComplete();
                updateFlowByScale (flow1, max_fee, fee);
                updateFlowByScale (flow2, max_fee, fee);
            }
        }
    }

    if (flow1.order.out != flow2.order.in)
        Throw<std::logic_error> ("Bridged flow is out of balance.");

    remaining_.out -= flow2.order.out;
    remaining_.in -= flow1.order.in;

    return std::make_pair (flow1, flow2);
}

//==============================================================================

Taker::Taker (CrossType cross_type, ApplyView& view,
    AccountID const& account, Amounts const& offer,
        std::uint32_t flags,
            beast::Journal journal)
    : BasicTaker (cross_type, account, offer, Quality(offer), flags,
                  calculateRate(view, offer.in.issue(), account),
                  calculateRate(view, offer.out.issue(), account), journal)
    , view_ (view)
    , m_flow_ (0)
    , direct_crossings_ (0)
    , bridge_crossings_ (0)
{
    assert (issue_in () == offer.in.issue ());
    assert (issue_out () == offer.out.issue ());

    auto const sle = view_.peek (keylet::account(issue_in().account));
    if (sle && sle->isFieldPresent(sfTransferFee))
    {
        totalFee_ = sle->getFieldAmount(sfTransferFee);
    }

    if (auto stream = journal_.debug())
    {
        stream << "Crossing as: " << to_string (account);

        if (isM (issue_in ()))
            stream << "   Offer in: " << format_amount (offer.in);
        else
            stream << "   Offer in: " << format_amount (offer.in) <<
                " (issuer: " << issue_in ().account << ")";

        if (isM (issue_out ()))
            stream << "  Offer out: " << format_amount (offer.out);
        else
            stream << "  Offer out: " << format_amount (offer.out) <<
                " (issuer: " << issue_out ().account << ")";

        stream <<
            "    Balance: " << format_amount (get_funds (account, offer.in));
    }
}

void
Taker::consume_offer (Offer& offer, Amounts const& order)
{
    if (order.in < zero)
        Throw<std::logic_error> ("flow with negative input.");

    if (order.out < zero)
        Throw<std::logic_error> ("flow with negative output.");

    JLOG(journal_.debug()) << "Consuming from offer " << offer;

    if (auto stream = journal_.trace())
    {
        auto const& available = offer.amount ();

        stream << "   in:" << format_amount (available.in);
        stream << "  out:" << format_amount(available.out);
    }

    offer.consume (view_, order);
}

STAmount
Taker::get_funds (AccountID const& account, STAmount const& amount) const
{
    return accountFunds(view_, account, amount, fhZERO_IF_FROZEN, journal_);
}

TER Taker::transferM (
    AccountID const& from,
    AccountID const& to,
    STAmount const& amount)
{
    if (!isM (amount))
        Throw<std::logic_error> ("Using transferM with IOU");

    if (from == to)
        return tesSUCCESS;

    // Transferring zero is equivalent to not doing a transfer
    if (amount == zero)
        return tesSUCCESS;

    return mtchain::transferM (view_, from, to, amount, journal_);
}

TER Taker::redeemIOU (
    AccountID const& account,
    STAmount const& amount,
    Issue const& issue)
{
    if (isM (amount))
        Throw<std::logic_error> ("Using redeemIOU with M");

    if (account == issue.account)
        return tesSUCCESS;

    switch (amount.signum ())
    {
    case 0: // Transferring zero is equivalent to not doing a transfer
        return tesSUCCESS;
    case -1:
        return tefEXCEPTION;
    }

    // If we are trying to redeem some amount, then the account
    // must have a credit balance.
    //if (get_funds (account, amount) <= zero)
    //    Throw<std::logic_error> ("redeemIOU has no funds to redeem");

    auto ret = mtchain::redeemIOU (view_, account, amount, issue, journal_);

    if (get_funds (account, amount) < zero)
        Throw<std::logic_error> ("redeemIOU redeemed more funds than available");

    return ret;
}

TER Taker::issueIOU (
    AccountID const& account,
    STAmount const& amount,
    Issue const& issue)
{
    if (isM (amount))
        Throw<std::logic_error> ("Using issueIOU with M");

    if (account == issue.account)
        return tesSUCCESS;

    // Transferring zero is equivalent to not doing a transfer
    if (amount == zero)
        return tesSUCCESS;

    return mtchain::issueIOU (view_, account, amount, issue, journal_);
}

STAmount BasicTaker::calcTxnFee (AccountID const& from, AccountID const& to,
                                 STAmount const& sell, STAmount const& buy,
                                 STAmount const& totalSell, STAmount const& totalBuy,
                                 STAmount const& totalFee, bool isSeller)
{
    STAmount fee = totalFee;
    //auto const sle = view_.peek (keylet::account(issue.account));
    //if (sle && sle->isFieldPresent(sfTransferFee))
    //fee = sle->getFieldAmount(sfTransferFee);

    if (fee.signum () > 0)
    {
        if (from == sell.issue().account || to == sell.issue().account)
        {
            return fee.zeroed();
        }

        if (isSeller)
        {
            if (totalSell.signum() > 0)
            {
                auto num = multiply(sell, fee, fee.issue());
                fee = divide (num, totalSell, fee.issue());
            }
            else
            {
                fee = fee.zeroed();
            }
        }
        else
        {
            if (totalBuy.signum() > 0)
            {
                auto num = multiply(buy, fee, fee.issue());
                fee = divide (num, totalBuy, fee.issue());
            }
            else
            {
                fee = fee.zeroed();
            }
        }
    }

    return fee;
}

TER Taker::chargeTxnFee (AccountID const& from, Issue const& issue, STAmount const& fee)
{
    TER result = tesSUCCESS;

    if (fee.signum () > 0)
    {
        if (isM(fee))
        {
            if (transferM(from, issue.account, fee) != tesSUCCESS)
            {
                result = tecUNFUNDED_PAYMENT_IOU_FEE;
            }
        }
        else
        {
            try {
                if (redeemIOU (from, fee, fee.issue()) != tesSUCCESS)
                {
                    result = tecNO_LINE_IOU_FEE;
                }
            } catch (std::exception const&)
            {
                result = tecUNFUNDED_PAYMENT_IOU_FEE;
            }

            if (result == tesSUCCESS)
            {
                result = issueIOU (issue.account, fee, fee.issue());
            }
        }
    }

    return result;
}

// Performs funds transfers to fill the given offer and adjusts offer.
TER
Taker::fill (BasicTaker::Flow const& flow, Offer& offer)
{
    TER result = tesSUCCESS;
    STAmount fee;

    if (cross_type () != CrossType::MToIou)
    {
        assert (!isM (flow.order.in));

        if(result == tesSUCCESS)
            result = redeemIOU (account (), flow.issuers.in, flow.issuers.in.issue ());

        if (result == tesSUCCESS)
            result = issueIOU (offer.owner (), flow.order.in, flow.order.in.issue ());

        if (result == tesSUCCESS && totalFee_.signum() > 0)
        {
            fee = calcTxnFee (account(), offer.owner(), flow.order.in, flow.order.out,
                              original_offer().in, original_offer().out, totalFee_, isSeller());
            result = chargeTxnFee (account (), issue_in (), fee);
            if (result == tesSUCCESS)
                addFee (fee);
        }
    }
    else
    {
        assert (isM (flow.order.in));

        if (result == tesSUCCESS)
            result = transferM (account (), offer.owner (), flow.order.in);
    }

    // Now send funds from the account whose offer we're taking
    if (cross_type () != CrossType::IouToM)
    {
        assert (!isM (flow.order.out));

        if(result == tesSUCCESS)
            result = redeemIOU (offer.owner (), flow.issuers.out, flow.issuers.out.issue ());

        if (result == tesSUCCESS)
            result = issueIOU (account (), flow.order.out, flow.order.out.issue ());

        if (result == tesSUCCESS && offer.getTotalFee().signum() > 0)
        {
            fee = calcTxnFee (offer.owner(), account(), flow.order.out, flow.order.in,
                              offer.original_amount().out, offer.original_amount().in,
                              offer.getTotalFee(), offer.isSeller());
            result = chargeTxnFee (offer.owner (), issue_out (), fee);
            if (result == tesSUCCESS)
                offer.setFee (fee);
        }
    }
    else
    {
        assert (isM (flow.order.out));

        if (result == tesSUCCESS)
            result = transferM (offer.owner (), account (), flow.order.out);
    }

    if (result == tesSUCCESS)
    {
        // adjust offer
        consume_offer (offer, flow.order);
        direct_crossings_++;
    }

    return result;
}

// Performs bridged funds transfers to fill the given offers and adjusts offers.
TER
Taker::fill (
    BasicTaker::Flow const& flow1, Offer& leg1,
    BasicTaker::Flow const& flow2, Offer& leg2)
{
    TER result = tesSUCCESS;
    STAmount fee;

    // Taker to leg1: IOU
    if (leg1.owner () != account ())
    {
        if (result == tesSUCCESS)
            result = redeemIOU (account (), flow1.issuers.in, flow1.issuers.in.issue ());

        if (result == tesSUCCESS)
            result = issueIOU (leg1.owner (), flow1.order.in, flow1.order.in.issue ());

        if (result == tesSUCCESS && totalFee_.signum() > 0)
        {
            fee = calcTxnFee (account(), leg1.owner(),  flow1.order.in, flow2.order.out,
                              original_offer().in, original_offer().out, totalFee_, isSeller());
            result = chargeTxnFee (account (), issue_in (), fee);
            if (result == tesSUCCESS)
                addFee (fee);
        }
    }

    // leg1 to leg2: bridging over M
    if (result == tesSUCCESS)
        result = transferM (leg1.owner (), leg2.owner (), flow1.order.out);

    // leg2 to Taker: IOU
    if (leg2.owner () != account ())
    {
        if (result == tesSUCCESS)
            result = redeemIOU (leg2.owner (), flow2.issuers.out, flow2.issuers.out.issue ());

        if (result == tesSUCCESS)
            result = issueIOU (account (), flow2.order.out, flow2.order.out.issue ());

        if (result == tesSUCCESS && leg2.getTotalFee().signum() > 0)
        {
            fee = calcTxnFee (leg2.owner(), account(), flow2.order.out, flow2.order.in,
                              leg2.original_amount().out, leg2.original_amount().in,
                              leg2.getTotalFee(), leg2.isSeller());
            result = chargeTxnFee (leg2.owner (), issue_out (), fee);
            if (result == tesSUCCESS)
                leg2.setFee (fee);
        }
    }

    if (result == tesSUCCESS)
    {
        // Adjust offers accordingly
        consume_offer (leg1, flow1.order);
        consume_offer (leg2, flow2.order);

        bridge_crossings_++;
        m_flow_ += flow1.order.out;
    }

    return result;
}

void BasicTaker::updateFlowByScale(BasicTaker::Flow& flow, STAmount const& num, STAmount const& den)
{
    flow.order.in  = multiply (flow.order.in,  num, flow.order.in.issue());
    flow.order.in  = divide   (flow.order.in,  den, flow.order.in.issue());
    flow.order.out = multiply (flow.order.out, num, flow.order.out.issue());
    flow.order.out = divide   (flow.order.out, den, flow.order.out.issue());

    flow.issuers.in  = multiply (flow.issuers.in,  num, flow.issuers.in.issue());
    flow.issuers.in  = divide   (flow.issuers.in,  den, flow.issuers.in.issue());
    flow.issuers.out = multiply (flow.issuers.out, num, flow.issuers.out.issue());
    flow.issuers.out = divide   (flow.issuers.out, den, flow.issuers.out.issue());
}

TER
Taker::cross (Offer& offer)
{
    // In direct crossings, at least one leg must not be M.
    if (isM (offer.amount ().in) && isM (offer.amount ().out))
        return tefINTERNAL;

    auto const amount = do_cross (offer);

    // If Offer.amount.in <= zero or Offer.amount.out <= zero , it should not be possible.
    if (amount.sanity_check())
    {
	if (amount.order.in == zero || amount.order.out == zero)
	{
		return tefINTERNAL;
	}	
    }

    return fill (amount, offer);
}

TER
Taker::cross (Offer& leg1, Offer& leg2)
{
    // In bridged crossings, M must can't be the input to the first leg
    // or the output of the second leg.
    if (isM (leg1.amount ().in) || isM (leg2.amount ().out))
        return tefINTERNAL;

    auto ret = do_cross (leg1, leg2);

    return fill (ret.first, leg1, ret.second, leg2);
}

} //

