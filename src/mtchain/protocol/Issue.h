//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_ISSUE_H_INCLUDED
#define MTCHAIN_PROTOCOL_ISSUE_H_INCLUDED

#include <cassert>
#include <functional>
#include <type_traits>

#include <mtchain/protocol/UintTypes.h>

namespace mtchain {

/** A currency issued by an account.
    @see Currency, AccountID, Issue, Book
*/
class Issue
{
public:
    Currency currency;
    AccountID account;

    Issue ()
    {
    }

    Issue (Currency const& c, AccountID const& a)
            : currency (c), account (a)
    {
    }
};

bool
isConsistent (Issue const& ac);

std::string
to_string (Issue const& ac);

std::ostream&
operator<< (std::ostream& os, Issue const& x);

template <class Hasher>
void
hash_append(Hasher& h, Issue const& r)
{
    using beast::hash_append;
    hash_append(h, r.currency, r.account);
}

/** Ordered comparison.
    The assets are ordered first by currency and then by account,
    if the currency is not M.
*/
int
compare (Issue const& lhs, Issue const& rhs);

/** Equality comparison. */
/** @{ */
bool
operator== (Issue const& lhs, Issue const& rhs);
bool
operator!= (Issue const& lhs, Issue const& rhs);
/** @} */

/** Strict weak ordering. */
/** @{ */
bool
operator< (Issue const& lhs, Issue const& rhs);
bool
operator> (Issue const& lhs, Issue const& rhs);
bool
operator>= (Issue const& lhs, Issue const& rhs);
bool
operator<= (Issue const& lhs, Issue const& rhs);
/** @} */

//------------------------------------------------------------------------------

/** Returns an asset specifier that represents M. */
inline Issue const& mIssue ()
{
    static Issue issue {mCurrency(), mAccount()};
    return issue;
}

/** Returns an asset specifier that represents no account and currency. */
inline Issue const& noIssue ()
{
    static Issue issue {noCurrency(), noAccount()};
    return issue;
}

}

#endif
