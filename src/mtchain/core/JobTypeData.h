//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CORE_JOBTYPEDATA_H_INCLUDED
#define MTCHAIN_CORE_JOBTYPEDATA_H_INCLUDED

#include <mtchain/basics/Log.h>
#include <mtchain/core/JobTypeInfo.h>
#include <mtchain/beast/insight/Collector.h>

namespace mtchain
{

struct JobTypeData
{
private:
    LoadMonitor m_load;

    /* Support for insight */
    beast::insight::Collector::ptr m_collector;

public:
    /* The job category which we represent */
    JobTypeInfo const& info;

    /* The number of jobs waiting */
    int waiting;

    /* The number presently running */
    int running;

    /* And the number we deferred executing because of job limits */
    int deferred;

    /* Notification callbacks */
    beast::insight::Event dequeue;
    beast::insight::Event execute;

    JobTypeData (JobTypeInfo const& info_,
            beast::insight::Collector::ptr const& collector, Logs& logs) noexcept
        : m_load (logs.journal ("LoadMonitor"))
        , m_collector (collector)
        , info (info_)
        , waiting (0)
        , running (0)
        , deferred (0)
    {
        m_load.setTargetLatency (
            info.getAverageLatency (),
            info.getPeakLatency());

        if (!info.special ())
        {
            dequeue = m_collector->make_event (info.name () + "_q");
            execute = m_collector->make_event (info.name ());
        }
    }

    /* Not copy-constructible or assignable */
    JobTypeData (JobTypeData const& other) = delete;
    JobTypeData& operator= (JobTypeData const& other) = delete;

    std::string name () const
    {
        return info.name ();
    }

    JobType type () const
    {
        return info.type ();
    }

    LoadMonitor& load ()
    {
        return m_load;
    }

    LoadMonitor::Stats stats ()
    {
        return m_load.getStats ();
    }
};

}

#endif
