//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/protocol/Issue.h>

namespace mtchain {

bool
isConsistent (Issue const& ac)
{
    return isM (ac.currency) == isM (ac.account);
}

std::string
to_string (Issue const& ac)
{
    if (isM (ac.account))
        return to_string (ac.currency);

    return to_string(ac.account) + "/" + to_string(ac.currency);
}

std::ostream&
operator<< (std::ostream& os, Issue const& x)
{
    os << to_string (x);
    return os;
}

/** Ordered comparison.
    The assets are ordered first by currency and then by account,
    if the currency is not M.
*/
int
compare (Issue const& lhs, Issue const& rhs)
{
    int diff = compare (lhs.currency, rhs.currency);
    if (diff != 0)
        return diff;
    if (isM (lhs.currency))
        return 0;
    return compare (lhs.account, rhs.account);
}

/** Equality comparison. */
/** @{ */
bool
operator== (Issue const& lhs, Issue const& rhs)
{
    return compare (lhs, rhs) == 0;
}

bool
operator!= (Issue const& lhs, Issue const& rhs)
{
    return ! (lhs == rhs);
}
/** @} */

/** Strict weak ordering. */
/** @{ */
bool
operator< (Issue const& lhs, Issue const& rhs)
{
    return compare (lhs, rhs) < 0;
}

bool
operator> (Issue const& lhs, Issue const& rhs)
{
    return rhs < lhs;
}

bool
operator>= (Issue const& lhs, Issue const& rhs)
{
    return ! (lhs < rhs);
}

bool
operator<= (Issue const& lhs, Issue const& rhs)
{
    return ! (rhs < lhs);
}

} //
