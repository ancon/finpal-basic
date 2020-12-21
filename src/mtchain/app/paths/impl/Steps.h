//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_IMPL_PAYSTEPS_H_INCLUDED
#define MTCHAIN_APP_PATHS_IMPL_PAYSTEPS_H_INCLUDED

#include <mtchain/app/paths/impl/AmountSpec.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/STLedgerEntry.h>
#include <mtchain/protocol/TER.h>

#include <boost/container/flat_set.hpp>
#include <boost/optional.hpp>

namespace mtchain {
class PaymentSandbox;
class ReadView;
class ApplyView;

/*
   A step in a payment path

   There are five concrete step classes:
     DirectStepI is an IOU step between accounts
     BookStepII is an IOU/IOU offer book
     BookStepIX is an IOU/M offer book
     BookStepXI is an M/IOU offer book
     MEndpointStep is the source or destination account for M

   Amounts may be transformed through a step in either the forward or the
   reverse direction. In the forward direction, the function `fwd` is used to
   find the amount the step would output given an input amount. In the reverse
   direction, the function `rev` is used to find the amount of input needed to
   produce the desired output.

   Amounts are always transformed using liquidity with the same quality (quality
   is the amount out/amount in). For example, a BookStep may use multiple offers
   when executing `fwd` or `rev`, but all those offers will be from the same
   quality directory.

   A step may not have enough liquidity to transform the entire requested
   amount. Both `fwd` and `rev` return a pair of amounts (one for input amount,
   one for output amount) that show how much of the requested amount the step
   was actually able to use.
 */
class Step
{
public:
    virtual ~Step () = default;

    /**
       Find the amount we need to put into the step to get the requested out
       subject to liquidity limits

       @param sb view with the strands state of balances and offers
       @param afView view the the state of balances before the strand runs
              this determines if an offer becomes unfunded or is found unfunded
       @param ofrsToRm offers found unfunded or in an error state are added to this collection
       @param out requested step output
       @return actual step input and output
     */
    virtual
    std::pair<EitherAmount, EitherAmount>
    rev (
        PaymentSandbox& sb,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        EitherAmount const& out) = 0;

    /**
       Find the amount we get out of the step given the input
       subject to liquidity limits

       @param sb view with the strands state of balances and offers
       @param afView view the the state of balances before the strand runs
              this determines if an offer becomes unfunded or is found unfunded
       @param ofrsToRm offers found unfunded or in an error state are added to this collection
       @param in requested step input
       @return actual step input and output
    */
    virtual
    std::pair<EitherAmount, EitherAmount>
    fwd (
        PaymentSandbox&,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        EitherAmount const& in) = 0;

    virtual
    boost::optional<EitherAmount>
    cachedIn () const = 0;

    virtual
    boost::optional<EitherAmount>
    cachedOut () const = 0;

    /**
       If this step is DirectStepI (IOU->IOU direct step), return the src
       account. This is needed for checkNoMTChain.
    */
    virtual boost::optional<AccountID>
    directStepSrcAcct () const
    {
        return boost::none;
    }

    /**
       If this step is a DirectStepI and the src redeems to the dst, return true,
       otherwise return false.
       If this step is a BookStep, return false if the owner pays the transfer fee,
       otherwise return true.
    */
    virtual bool
    redeems (ReadView const& sb, bool fwd) const
    {
        return false;
    }

    /** If this step is a DirectStepI, return the quality in of the dst account.
     */
    virtual std::uint32_t
    lineQualityIn (ReadView const&) const
    {
        return QUALITY_ONE;
    }

    /**
       If this step is a BookStep, return the book.
    */
    virtual boost::optional<Book>
    bookStepBook () const
    {
        return boost::none;
    }

    /**
       Check if amount is zero
    */
    virtual
    bool
    dry (EitherAmount const& out) const = 0;

    virtual
    bool
    equalOut (
        EitherAmount const& lhs,
        EitherAmount const& rhs) const = 0;

    virtual bool equalIn (
        EitherAmount const& lhs,
        EitherAmount const& rhs) const = 0;

    /**
       Check that the step can correctly execute in the forward direction

       @param sb view with the strands state of balances and offers
       @param afView view the the state of balances before the strand runs
       this determines if an offer becomes unfunded or is found unfunded
       @param in requested step input
       @return first element is true is step is valid, second element is out amount
    */
    virtual
    std::pair<bool, EitherAmount>
    validFwd (
        PaymentSandbox& sb,
        ApplyView& afView,
        EitherAmount const& in) = 0;

    friend bool operator==(Step const& lhs, Step const& rhs)
    {
        return lhs.equal (rhs);
    }

    friend bool operator!=(Step const& lhs, Step const& rhs)
    {
        return ! (lhs == rhs);
    }

    friend
    std::ostream&
    operator << (
        std::ostream& stream,
        Step const& step)
    {
        stream << step.logString ();
        return stream;
    }
private:
    virtual
    std::string
    logString () const = 0;

    virtual bool equal (Step const& rhs) const = 0;
};

using Strand = std::vector<std::unique_ptr<Step>>;

inline
bool operator==(Strand const& lhs, Strand const& rhs)
{
    if (lhs.size () != rhs.size ())
        return false;
    for (size_t i = 0, e = lhs.size (); i != e; ++i)
        if (*lhs[i] != *rhs[i])
            return false;
    return true;
}

/*
   Create a strand for the specified path

   @param sb view for trust lines, balances, and attributes like auth and freeze
   @param src Account that is sending assets
   @param dst Account that is receiving assets
   @param deliver Asset the dst account will receive
   (if issuer of deliver == dst, then accept any issuer)
   @param sendMax Optional asset to send.
   @param path Liquidity sources to use for this strand of the payment. The path
               contains an ordered collection of the offer books to use and
               accounts to mtchain through.
   @param addDefaultPath Determines if the default path should be considered
   @param l logs to write journal messages to
   @return error code and collection of strands
*/
std::pair<TER, Strand>
toStrand (
    ReadView const& sb,
    AccountID const& src,
    AccountID const& dst,
    Issue const& deliver,
    boost::optional<Issue> const& sendMaxIssue,
    STPath const& path,
    bool ownerPaysTransferFee,
    beast::Journal j);

/**
   Create a strand for each specified path (including the default path, if
   specified)

   @param sb view for trust lines, balances, and attributes like auth and freeze
   @param src Account that is sending assets
   @param dst Account that is receiving assets
   @param deliver Asset the dst account will receive
                  (if issuer of deliver == dst, then accept any issuer)
   @param sendMax Optional asset to send.
   @param paths Paths to use to fullfill the payment. Each path in the pathset
                contains an ordered collection of the offer books to use and
                accounts to mtchain through.
   @param addDefaultPath Determines if the default path should be considered
   @param l logs to write journal messages to
   @return error code and collection of strands
*/
std::pair<TER, std::vector<Strand>>
toStrands (ReadView const& sb,
    AccountID const& src,
    AccountID const& dst,
    Issue const& deliver,
    boost::optional<Issue> const& sendMax,
    STPathSet const& paths,
    bool addDefaultPath,
    bool ownerPaysTransferFee,
    beast::Journal j);

template <class TIn, class TOut, class TDerived>
struct StepImp : public Step
{
    std::pair<EitherAmount, EitherAmount>
    rev (
        PaymentSandbox& sb,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        EitherAmount const& out) override
    {
        auto const r =
            static_cast<TDerived*> (this)->revImp (sb, afView, ofrsToRm, get<TOut>(out));
        return {EitherAmount (r.first), EitherAmount (r.second)};
    }

    // Given the requested amount to consume, compute the amount produced.
    // Return the consumed/produced
    std::pair<EitherAmount, EitherAmount>
    fwd (
        PaymentSandbox& sb,
        ApplyView& afView,
        boost::container::flat_set<uint256>& ofrsToRm,
        EitherAmount const& in) override
    {
        auto const r =
            static_cast<TDerived*> (this)->fwdImp (sb, afView, ofrsToRm, get<TIn>(in));
        return {EitherAmount (r.first), EitherAmount (r.second)};
    }

    bool
    dry (EitherAmount const& out) const override
    {
        return get<TOut>(out) == beast::zero;
    }

    bool
    equalOut (EitherAmount const& lhs, EitherAmount const& rhs) const override
    {
        return get<TOut> (lhs) == get<TOut> (rhs);
    }

    bool
    equalIn (EitherAmount const& lhs, EitherAmount const& rhs) const override
    {
        return get<TIn> (lhs) == get<TIn> (rhs);
    }
};

// Thrown when unexpected errors occur
class FlowException : public std::runtime_error
{
public:
    TER ter;
    std::string msg;

    FlowException (TER t, std::string const& msg)
        : std::runtime_error (msg)
        , ter (t)
    {
    }

    explicit
    FlowException (TER t)
        : std::runtime_error (transHuman (t))
        , ter (t)
    {
    }
};

// Check equal with tolerance
bool checkNear (IOUAmount const& expected, IOUAmount const& actual);
bool checkNear (MAmount const& expected, MAmount const& actual);

/**
   Context needed for error checking
 */
struct StrandContext
{
    ReadView const& view;
    AccountID const strandSrc;
    AccountID const strandDst;
    bool const isFirst;
    bool const isLast = false;
    bool ownerPaysTransferFee;
    size_t const strandSize;
    // The previous step in the strand. Needed to check the no mtchain constraint
    Step const* const prevStep = nullptr;
    // A strand may not include the same account node more than once
    // in the same currency. In a direct step, an account will show up
    // at most twice: once as a src and once as a dst (hence the two element array).
    // The strandSrc and strandDst will only show up once each.
    std::array<boost::container::flat_set<Issue>, 2>& seenDirectIssues;
    // A strand may not include an offer that output the same issue more than once
    boost::container::flat_set<Issue>& seenBookOuts;
    beast::Journal j;

    StrandContext (ReadView const& view_,
        std::vector<std::unique_ptr<Step>> const& strand_,
        // A strand may not include an inner node that
        // replicates the source or destination.
        AccountID strandSrc_,
        AccountID strandDst_,
        bool isLast_,
        bool ownerPaysTransferFee_,
        std::array<boost::container::flat_set<Issue>, 2>& seenDirectIssues_,
        boost::container::flat_set<Issue>& seenBookOuts_,
        beast::Journal j);
};

namespace test {
// Needed for testing
bool directStepEqual (Step const& step,
    AccountID const& src,
    AccountID const& dst,
    Currency const& currency);

bool mEndpointStepEqual (Step const& step, AccountID const& acc);

bool bookStepEqual (Step const& step, mtchain::Book const& book);
}

std::pair<TER, std::unique_ptr<Step>>
make_DirectStepI (
    StrandContext const& ctx,
    AccountID const& src,
    AccountID const& dst,
    Currency const& c);

std::pair<TER, std::unique_ptr<Step>>
make_BookStepII (
    StrandContext const& ctx,
    Issue const& in,
    Issue const& out);

std::pair<TER, std::unique_ptr<Step>>
make_BookStepIX (
    StrandContext const& ctx,
    Issue const& in);

std::pair<TER, std::unique_ptr<Step>>
make_BookStepXI (
    StrandContext const& ctx,
    Issue const& out);

std::pair<TER, std::unique_ptr<Step>>
make_MEndpointStep (
    StrandContext const& ctx,
    AccountID const& acc);

template<class InAmt, class OutAmt>
bool
isDirectMToM(Strand const& strand);

} //

#endif
