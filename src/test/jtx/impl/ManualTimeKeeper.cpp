//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <test/jtx/ManualTimeKeeper.h>

namespace mtchain {
namespace test {

using namespace std::chrono_literals;

ManualTimeKeeper::ManualTimeKeeper()
    : closeOffset_ {}
    , now_ (0s)
{
}

void
ManualTimeKeeper::run (std::vector<std::string> const& servers)
{
}

auto
ManualTimeKeeper::now() const ->
    time_point
{
    std::lock_guard<std::mutex> lock(mutex_);
    return now_;
}
    
auto
ManualTimeKeeper::closeTime() const ->
    time_point
{
    std::lock_guard<std::mutex> lock(mutex_);
    return now_ + closeOffset_;
}

void
ManualTimeKeeper::adjustCloseTime(
    std::chrono::duration<std::int32_t> amount)
{
    // Copied from TimeKeeper::adjustCloseTime
    using namespace std::chrono;
    auto const s = amount.count();
    std::lock_guard<std::mutex> lock(mutex_);
    // Take large offsets, ignore small offsets,
    // push the close time towards our wall time.
    if (s > 1)
        closeOffset_ += seconds((s + 3) / 4);
    else if (s < -1)
        closeOffset_ += seconds((s - 3) / 4);
    else
        closeOffset_ = (closeOffset_ * 3) / 4;
}

std::chrono::duration<std::int32_t>
ManualTimeKeeper::nowOffset() const
{
    return {};
}

std::chrono::duration<std::int32_t>
ManualTimeKeeper::closeOffset() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return closeOffset_;
}

void
ManualTimeKeeper::set (time_point now)
{
    std::lock_guard<std::mutex> lock(mutex_);
    now_ = now;
}

auto
ManualTimeKeeper::adjust(
        std::chrono::system_clock::time_point when) ->
    time_point
{
    return time_point(
        std::chrono::duration_cast<duration>(
            when.time_since_epoch() -
                days(10957)));
}
} // test
} //
