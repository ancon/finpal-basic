//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CORE_LOADMONITOR_H_INCLUDED
#define MTCHAIN_CORE_LOADMONITOR_H_INCLUDED

#include <mtchain/core/LoadEvent.h>
#include <mtchain/beast/utility/Journal.h>
#include <chrono>
#include <mutex>

namespace mtchain {

// Monitors load levels and response times

// VFALCO TODO Rename this. Having both LoadManager and LoadMonitor is confusing.
//
class LoadMonitor
{
public:
    explicit
    LoadMonitor (beast::Journal j);

    void addLoadSample (LoadEvent const& sample);

    void addSamples (int count, std::chrono::milliseconds latency);

    void setTargetLatency (std::uint64_t avg, std::uint64_t pk);

    bool isOverTarget (std::uint64_t avg, std::uint64_t peak);

    // VFALCO TODO make this return the values in a struct.
    struct Stats
    {
        Stats();

        std::uint64_t count;
        std::uint64_t latencyAvg;
        std::uint64_t latencyPeak;
        bool isOverloaded;
    };

    Stats getStats ();

    bool isOver ();

private:
    void update ();

    std::mutex mutex_;

    std::uint64_t mCounts;
    int           mLatencyEvents;
    std::uint64_t mLatencyMSAvg;
    std::uint64_t mLatencyMSPeak;
    std::uint64_t mTargetLatencyAvg;
    std::uint64_t mTargetLatencyPk;
    int           mLastUpdate;
    beast::Journal j_;
};

} //

#endif
