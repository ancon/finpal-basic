//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_AMOUNTCONVERSION_H_INCLUDED
#define MTCHAIN_PROTOCOL_AMOUNTCONVERSION_H_INCLUDED

#include <mtchain/protocol/IOUAmount.h>
#include <mtchain/protocol/MAmount.h>
#include <mtchain/protocol/STAmount.h>

namespace mtchain {

inline
STAmount
toSTAmount (IOUAmount const& iou, Issue const& iss)
{
    bool const isNeg = iou.signum() < 0;
    std::uint64_t const umant = isNeg ? - iou.mantissa () : iou.mantissa ();
    return STAmount (iss, umant, iou.exponent (), /*native*/ false, isNeg,
                     STAmount::unchecked ());
}

inline
STAmount
toSTAmount (IOUAmount const& iou)
{
    return toSTAmount (iou, noIssue ());
}

inline
STAmount
toSTAmount (MAmount const& m)
{
    bool const isNeg = m.signum() < 0;
    std::uint64_t const umant = isNeg ? - m.drops () : m.drops ();
    return STAmount (umant, isNeg);
}

inline
STAmount
toSTAmount (MAmount const& m, Issue const& iss)
{
    assert (isM(iss.account) && isM(iss.currency));
    return toSTAmount (m);
}

template <class T>
T
toAmount (STAmount const& amt)
{
    static_assert(sizeof(T) == -1, "Must use specialized function");
    return T(0);
}

template <>
inline
STAmount
toAmount<STAmount> (STAmount const& amt)
{
    return amt;
}

template <>
inline
IOUAmount
toAmount<IOUAmount> (STAmount const& amt)
{
    assert (amt.mantissa () < std::numeric_limits<std::int64_t>::max ());
    bool const isNeg = amt.negative ();
    std::int64_t const sMant =
            isNeg ? - std::int64_t (amt.mantissa ()) : amt.mantissa ();

    assert (! isM (amt));
    return IOUAmount (sMant, amt.exponent ());
}

template <>
inline
MAmount
toAmount<MAmount> (STAmount const& amt)
{
    assert (amt.mantissa () < std::numeric_limits<std::int64_t>::max ());
    bool const isNeg = amt.negative ();
    std::int64_t const sMant =
            isNeg ? - std::int64_t (amt.mantissa ()) : amt.mantissa ();

    assert (isM (amt));
    return MAmount (sMant);
}


}

#endif
