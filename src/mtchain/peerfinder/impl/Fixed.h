//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_FIXED_H_INCLUDED
#define MTCHAIN_PEERFINDER_FIXED_H_INCLUDED

#include <mtchain/peerfinder/impl/Tuning.h>

namespace mtchain {
namespace PeerFinder {

/** Metadata for a Fixed slot. */
class Fixed
{
public:
    explicit Fixed (clock_type& clock)
        : m_when (clock.now ())
        , m_failures (0)
    {
    }

    Fixed (Fixed const&) = default;

    /** Returns the time after which we shoud allow a connection attempt. */
    clock_type::time_point const& when () const
    {
        return m_when;
    }

    /** Updates metadata to reflect a failed connection. */
    void failure (clock_type::time_point const& now)
    {
        m_failures = std::min (m_failures + 1,
            Tuning::connectionBackoff.size() - 1);
        m_when = now + std::chrono::minutes (
            Tuning::connectionBackoff [m_failures]);
    }

    /** Updates metadata to reflect a successful connection. */
    void success (clock_type::time_point const& now)
    {
        m_failures = 0;
        m_when = now;
    }

private:
    clock_type::time_point m_when;
    std::size_t m_failures;
};

}
}

#endif
