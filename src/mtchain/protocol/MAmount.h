//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_MAMOUNT_H_INCLUDED
#define MTCHAIN_PROTOCOL_MAMOUNT_H_INCLUDED

#include <mtchain/basics/contract.h>
#include <mtchain/protocol/SystemParameters.h>
#include <mtchain/beast/utility/Zero.h>
#include <boost/operators.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <string>
#include <type_traits>

using beast::zero;

namespace mtchain {

class MAmount
    : private boost::totally_ordered <MAmount>
    , private boost::additive <MAmount>
{
private:
    std::int64_t drops_;

public:
    MAmount () = default;
    MAmount (MAmount const& other) = default;
    MAmount& operator= (MAmount const& other) = default;

    MAmount (beast::Zero)
        : drops_ (0)
    {
    }

    MAmount&
    operator= (beast::Zero)
    {
        drops_ = 0;
        return *this;
    }

    template <class Integer,
        class = typename std::enable_if_t <
            std::is_integral<Integer>::value>>
    MAmount (Integer drops)
        : drops_ (static_cast<std::int64_t> (drops))
    {
    }

    template <class Integer,
        class = typename std::enable_if_t <
            std::is_integral<Integer>::value>>
    MAmount&
    operator= (Integer drops)
    {
        drops_ = static_cast<std::int64_t> (drops);
        return *this;
    }

    MAmount&
    operator+= (MAmount const& other)
    {
        drops_ += other.drops_;
        return *this;
    }

    MAmount&
    operator-= (MAmount const& other)
    {
        drops_ -= other.drops_;
        return *this;
    }

    MAmount
    operator- () const
    {
        return { -drops_ };
    }

    bool
    operator==(MAmount const& other) const
    {
        return drops_ == other.drops_;
    }

    bool
    operator<(MAmount const& other) const
    {
        return drops_ < other.drops_;
    }

    /** Returns true if the amount is not zero */
    explicit
    operator bool() const noexcept
    {
        return drops_ != 0;
    }

    /** Return the sign of the amount */
    int
    signum() const noexcept
    {
        return (drops_ < 0) ? -1 : (drops_ ? 1 : 0);
    }

    /** Returns the number of drops */
    std::int64_t
    drops () const
    {
        return drops_;
    }
};

inline
std::string
to_string (MAmount const& amount)
{
    return std::to_string (amount.drops ());
}

inline
MAmount
mulRatio (
    MAmount const& amt,
    std::uint32_t num,
    std::uint32_t den,
    bool roundUp)
{
    using namespace boost::multiprecision;

    if (!den)
        Throw<std::runtime_error> ("division by zero");

    int128_t const amt128 (amt.drops ());
    auto const neg = amt.drops () < 0;
    auto const m = amt128 * num;
    auto r = m / den;
    if (m % den)
    {
        if (!neg && roundUp)
            r += 1;
        if (neg && !roundUp)
            r -= 1;
    }
    if (r > std::numeric_limits<std::int64_t>::max ())
        Throw<std::overflow_error> ("M mulRatio overflow");
    return MAmount (r.convert_to<std::int64_t> ());
}

/** Returns true if the amount does not exceed the initial M in existence. */
inline
bool isLegalAmount (MAmount const& amount)
{
    return amount.drops () <= SYSTEM_CURRENCY_START;
}

}

#endif
