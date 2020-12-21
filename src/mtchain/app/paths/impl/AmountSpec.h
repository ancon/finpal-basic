//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PATH_IMPL_AMOUNTSPEC_H_INCLUDED
#define MTCHAIN_PATH_IMPL_AMOUNTSPEC_H_INCLUDED

#include <mtchain/protocol/IOUAmount.h>
#include <mtchain/protocol/MAmount.h>
#include <mtchain/protocol/STAmount.h>

namespace mtchain {

struct AmountSpec
{
    bool native;
    union
    {
        MAmount m;
        IOUAmount iou;
    };
    boost::optional<AccountID> issuer;
    boost::optional<Currency> currency;

    friend
    std::ostream&
    operator << (
        std::ostream& stream,
        AmountSpec const& amt)
    {
        if (amt.native)
            stream << to_string (amt.m);
        else
            stream << to_string (amt.iou);
        if (amt.currency)
            stream << "/(" << *amt.currency << ")";
        if (amt.issuer)
            stream << "/" << *amt.issuer << "";
        return stream;
    }
};

struct EitherAmount
{
#ifndef NDEBUG
    bool native = false;
#endif

    union
    {
        IOUAmount iou;
        MAmount m;
    };

    EitherAmount () = default;

    explicit
    EitherAmount (IOUAmount const& a)
            :iou(a)
    {
    }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
    // ignore warning about half of iou amount being uninitialized
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    explicit
    EitherAmount (MAmount const& a)
            :m(a)
    {
#ifndef NDEBUG
        native = true;
#endif
    }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    explicit
    EitherAmount (AmountSpec const& a)
    {
#ifndef NDEBUG
        native = a.native;
#endif
        if (a.native)
            m = a.m;
        else
            iou = a.iou;
    }
};

template <class T>
T&
get (EitherAmount& amt)
{
    static_assert(sizeof(T) == -1, "Must used specialized function");
    return T(0);
}

template <>
inline
IOUAmount&
get<IOUAmount> (EitherAmount& amt)
{
    assert (!amt.native);
    return amt.iou;
}

template <>
inline
MAmount&
get<MAmount> (EitherAmount& amt)
{
    assert (amt.native);
    return amt.m;
}

template <class T>
T const&
get (EitherAmount const& amt)
{
    static_assert(sizeof(T) == -1, "Must used specialized function");
    return T(0);
}

template <>
inline
IOUAmount const&
get<IOUAmount> (EitherAmount const& amt)
{
    assert (!amt.native);
    return amt.iou;
}

template <>
inline
MAmount const&
get<MAmount> (EitherAmount const& amt)
{
    assert (amt.native);
    return amt.m;
}

inline
AmountSpec
toAmountSpec (STAmount const& amt)
{
    assert (amt.mantissa () < std::numeric_limits<std::int64_t>::max ());
    bool const isNeg = amt.negative ();
    std::int64_t const sMant =
        isNeg ? - std::int64_t (amt.mantissa ()) : amt.mantissa ();
    AmountSpec result;

    result.native = isM (amt);
    if (result.native)
    {
        result.m = MAmount (sMant);
    }
    else
    {
        result.iou = IOUAmount (sMant, amt.exponent ());
        result.issuer = amt.issue().account;
        result.currency = amt.issue().currency;
    }

    return result;
}

inline
EitherAmount
toEitherAmount (STAmount const& amt)
{
    if (isM (amt))
        return EitherAmount{amt.m()};
    return EitherAmount{amt.iou()};
}

inline
AmountSpec
toAmountSpec (
    EitherAmount const& ea,
    boost::optional<Currency> const& c)
{
    AmountSpec r;
    r.native = (!c || isM (*c));
    r.currency = c;
    assert (ea.native == r.native);
    if (r.native)
    {
        r.m = ea.m;
    }
    else
    {
        r.iou = ea.iou;
    }
    return r;
}

}

#endif
