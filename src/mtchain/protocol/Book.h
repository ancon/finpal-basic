//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_BOOK_H_INCLUDED
#define MTCHAIN_PROTOCOL_BOOK_H_INCLUDED

#include <mtchain/protocol/Issue.h>
#include <boost/utility/base_from_member.hpp>

namespace mtchain {

/** Specifies an order book.
    The order book is a pair of Issues called in and out.
    @see Issue.
*/
class Book
{
public:
    Issue in;
    Issue out;

    Book ()
    {
    }

    Book (Issue const& in_, Issue const& out_)
        : in (in_)
        , out (out_)
    {
    }
};

bool
isConsistent (Book const& book);

std::string
to_string (Book const& book);

std::ostream&
operator<< (std::ostream& os, Book const& x);

template <class Hasher>
void
hash_append (Hasher& h, Book const& b)
{
    using beast::hash_append;
    hash_append(h, b.in, b.out);
}

Book
reversed (Book const& book);

/** Ordered comparison. */
int
compare (Book const& lhs, Book const& rhs);

/** Equality comparison. */
/** @{ */
bool
operator== (Book const& lhs, Book const& rhs);
bool
operator!= (Book const& lhs, Book const& rhs);
/** @} */

/** Strict weak ordering. */
/** @{ */
bool
operator< (Book const& lhs,Book const& rhs);
bool
operator> (Book const& lhs, Book const& rhs);
bool
operator>= (Book const& lhs, Book const& rhs);
bool
operator<= (Book const& lhs, Book const& rhs);
/** @} */

}

//------------------------------------------------------------------------------

namespace std {

template <>
struct hash <mtchain::Issue>
    : private boost::base_from_member <std::hash <mtchain::Currency>, 0>
    , private boost::base_from_member <std::hash <mtchain::AccountID>, 1>
{
private:
    using currency_hash_type = boost::base_from_member <
        std::hash <mtchain::Currency>, 0>;
    using issuer_hash_type = boost::base_from_member <
        std::hash <mtchain::AccountID>, 1>;

public:
    using value_type = std::size_t;
    using argument_type = mtchain::Issue;

    value_type operator() (argument_type const& value) const
    {
        value_type result (currency_hash_type::member (value.currency));
        if (!isM (value.currency))
            boost::hash_combine (result,
                issuer_hash_type::member (value.account));
        return result;
    }
};

//------------------------------------------------------------------------------

template <>
struct hash <mtchain::Book>
{
private:
    using hasher = std::hash <mtchain::Issue>;

    hasher m_hasher;

public:
    using value_type = std::size_t;
    using argument_type = mtchain::Book;

    value_type operator() (argument_type const& value) const
    {
        value_type result (m_hasher (value.in));
        boost::hash_combine (result, m_hasher (value.out));
        return result;
    }
};

}

//------------------------------------------------------------------------------

namespace boost {

template <>
struct hash <mtchain::Issue>
    : std::hash <mtchain::Issue>
{
    using Base = std::hash <mtchain::Issue>;
    // VFALCO NOTE broken in vs2012
    //using Base::Base; // inherit ctors
};

template <>
struct hash <mtchain::Book>
    : std::hash <mtchain::Book>
{
    using Base = std::hash <mtchain::Book>;
    // VFALCO NOTE broken in vs2012
    //using Base::Base; // inherit ctors
};

}

#endif
