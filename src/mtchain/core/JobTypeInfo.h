//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CORE_JOBTYPEINFO_H_INCLUDED
#define MTCHAIN_CORE_JOBTYPEINFO_H_INCLUDED

namespace mtchain
{

/** Holds all the 'static' information about a job, which does not change */
class JobTypeInfo
{
private:
    JobType const m_type;
    std::string const m_name;

    /** The limit on the number of running jobs for this job type. */
    int const m_limit;

    /** Special jobs are not dispatched via the job queue */
    bool const m_special;

    /** Average and peak latencies for this job type. 0 is none specified */
    std::uint64_t const m_avgLatency;
    std::uint64_t const m_peakLatency;

public:
    // Not default constructible
    JobTypeInfo () = delete;

    JobTypeInfo (JobType type, std::string name, int limit,
            bool special, std::uint64_t avgLatency, std::uint64_t peakLatency)
        : m_type (type)
        , m_name (name)
        , m_limit (limit)
        , m_special (special)
        , m_avgLatency (avgLatency)
        , m_peakLatency (peakLatency)
    {

    }

    JobType type () const
    {
        return m_type;
    }

    std::string name () const
    {
        return m_name;
    }

    int limit () const
    {
        return m_limit;
    }

    bool special () const
    {
        return m_special;
    }

    std::uint64_t getAverageLatency () const
    {
        return m_avgLatency;
    }

    std::uint64_t getPeakLatency () const
    {
        return m_peakLatency;
    }
};

}

#endif
