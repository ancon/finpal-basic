//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PEERS_CLUSTERNODESTATUS_H_INCLUDED
#define MTCHAIN_APP_PEERS_CLUSTERNODESTATUS_H_INCLUDED

#include <mtchain/basics/chrono.h>
#include <mtchain/protocol/PublicKey.h>
#include <cstdint>
#include <string>

namespace mtchain {

class ClusterNode
{
public:
    ClusterNode() = delete;

    ClusterNode(
            PublicKey const& identity,
            std::string const& name,
            std::uint32_t fee = 0,
            NetClock::time_point rtime = NetClock::time_point{})
        : identity_ (identity)
        , name_(name)
        , mLoadFee(fee)
        , mReportTime(rtime)
    { }

    std::string const& name() const
    {
        return name_;
    }

    std::uint32_t getLoadFee() const
    {
        return mLoadFee;
    }

    NetClock::time_point getReportTime() const
    {
        return mReportTime;
    }

    PublicKey const&
    identity () const
    {
        return identity_;
    }

private:
    PublicKey const identity_;
    std::string name_;
    std::uint32_t mLoadFee = 0;
    NetClock::time_point mReportTime = {};
};

} //

#endif
