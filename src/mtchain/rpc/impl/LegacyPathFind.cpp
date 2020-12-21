//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/LoadFeeTrack.h>
#include <mtchain/core/Job.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/rpc/impl/LegacyPathFind.h>
#include <mtchain/rpc/impl/Tuning.h>

namespace mtchain {
namespace RPC {

LegacyPathFind::LegacyPathFind (bool isAdmin, Application& app) : m_isOk (false)
{
    if (isAdmin)
    {
        ++inProgress;
        m_isOk = true;
        return;
    }

    auto const& jobCount = app.getJobQueue ().getJobCountGE (jtCLIENT);
    if (jobCount > Tuning::maxPathfindJobCount || app.getFeeTrack().isLoadedLocal ())
        return;

    while (true)
    {
        int prevVal = inProgress.load();
        if (prevVal >= Tuning::maxPathfindsInProgress)
            return;

        if (inProgress.compare_exchange_strong (
                prevVal,
                prevVal + 1,
                std::memory_order_release,
                std::memory_order_relaxed))
        {
            m_isOk = true;
            return;
        }
    }
}

LegacyPathFind::~LegacyPathFind ()
{
    if (m_isOk)
        --inProgress;
}

std::atomic <int> LegacyPathFind::inProgress (0);

} // RPC
} //
