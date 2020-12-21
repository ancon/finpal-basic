//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerTiming.h>
#include <mtchain/basics/Log.h>
#include <algorithm>
#include <iterator>

namespace mtchain {

NetClock::duration
getNextLedgerTimeResolution (
    NetClock::duration previousResolution,
    bool previousAgree,
    std::uint32_t ledgerSeq)
{
    assert (ledgerSeq);

    using namespace std::chrono;
    // Find the current resolution:
    auto iter = std::find (std::begin (ledgerPossibleTimeResolutions),
        std::end (ledgerPossibleTimeResolutions), previousResolution);
    assert (iter != std::end (ledgerPossibleTimeResolutions));

    // This should never happen, but just as a precaution
    if (iter == std::end (ledgerPossibleTimeResolutions))
        return previousResolution;

    // If we did not previously agree, we try to decrease the resolution to
    // improve the chance that we will agree now.
    if (!previousAgree && ledgerSeq % decreaseLedgerTimeResolutionEvery == 0)
    {
        if (++iter != std::end (ledgerPossibleTimeResolutions))
            return *iter;
    }

    // If we previously agreed, we try to increase the resolution to determine
    // if we can continue to agree.
    if (previousAgree && ledgerSeq % increaseLedgerTimeResolutionEvery == 0)
    {
        if (iter-- != std::begin (ledgerPossibleTimeResolutions))
            return *iter;
    }

    return previousResolution;
}

NetClock::time_point
roundCloseTime (
    NetClock::time_point closeTime,
    NetClock::duration closeResolution)
{
    if (closeTime == NetClock::time_point{})
        return closeTime;

    closeTime += (closeResolution / 2);
    return closeTime - (closeTime.time_since_epoch() % closeResolution);
}

bool
shouldCloseLedger (
    bool anyTransactions,
    int previousProposers,
    int proposersClosed,
    int proposersValidated,
    std::chrono::milliseconds previousTime,
    std::chrono::milliseconds currentTime, // Time since last ledger's close time
    std::chrono::milliseconds openTime,    // Time waiting to close this ledger
    std::chrono::seconds idleInterval,
    beast::Journal j)
{
    using namespace std::chrono_literals;
    if ((previousTime < -1s) || (previousTime > 10min) ||
        (currentTime > 10min))
    {
        // These are unexpected cases, we just close the ledger
        JLOG (j.warn()) <<
            "shouldCloseLedger Trans=" << (anyTransactions ? "yes" : "no") <<
            " Prop: " << previousProposers << "/" << proposersClosed <<
            " Secs: " << currentTime.count() << " (last: " <<
            previousTime.count() << ")";
        return true;
    }

    if ((proposersClosed + proposersValidated) > (previousProposers / 2))
    {
        // If more than half of the network has closed, we close
        JLOG (j.trace()) << "Others have closed";
        return true;
    }

    if (!anyTransactions)
    {
        // Only close at the end of the idle interval
        return currentTime >= idleInterval; // normal idle
    }

    // Preserve minimum ledger open time
    if (openTime < LEDGER_MIN_CLOSE)
    {
        JLOG (j.debug()) <<
            "Must wait minimum time before closing";
        return false;
    }

    // Don't let this ledger close more than twice as fast as the previous
    // ledger reached consensus so that slower validators can slow down
    // the network
    if (openTime < (previousTime / 2))
    {
        JLOG (j.debug()) <<
            "Ledger has not been open long enough";
        return false;
    }

    // Close the ledger
    return true;
}

bool
checkConsensusReached (int agreeing, int total, bool count_self)
{
    // If we are alone, we have a consensus
    if (total == 0)
        return true;

    if (count_self)
    {
        ++agreeing;
        ++total;
    }

    int currentPercentage = (agreeing * 100) / total;

    return currentPercentage > minimumConsensusPercentage;
}

ConsensusState
checkConsensus (
    int previousProposers,
    int currentProposers,
    int currentAgree,
    int currentFinished,
    std::chrono::milliseconds previousAgreeTime,
    std::chrono::milliseconds currentAgreeTime,
    bool proposing,
    beast::Journal j)
{
    JLOG (j.trace()) <<
        "checkConsensus: prop=" << currentProposers <<
        "/" << previousProposers <<
        " agree=" << currentAgree << " validated=" << currentFinished <<
        " time=" << currentAgreeTime.count() <<  "/" << previousAgreeTime.count();

    if (currentAgreeTime <= LEDGER_MIN_CONSENSUS)
        return ConsensusState::No;

    if (currentProposers < (previousProposers * 3 / 4))
    {
        // Less than 3/4 of the last ledger's proposers are present; don't
        // rush: we may need more time.
        if (currentAgreeTime < (previousAgreeTime + LEDGER_MIN_CONSENSUS))
        {
            JLOG (j.trace()) <<
                "too fast, not enough proposers";
            return ConsensusState::No;
        }
    }

    // Have we, together with the nodes on our UNL list, reached the threshold
    // to declare consensus?
    if (checkConsensusReached (currentAgree, currentProposers, proposing))
    {
        JLOG (j.debug()) << "normal consensus";
        return ConsensusState::Yes;
    }

    // Have sufficient nodes on our UNL list moved on and reached the threshold
    // to declare consensus?
    if (checkConsensusReached (currentFinished, currentProposers, false))
    {
        JLOG (j.warn()) <<
            "We see no consensus, but 80% of nodes have moved on";
        return ConsensusState::MovedOn;
    }

    // no consensus yet
    JLOG (j.trace()) << "no consensus";
    return ConsensusState::No;
}

} //
