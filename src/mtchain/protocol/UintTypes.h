//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_UINTTYPES_H_INCLUDED
#define MTCHAIN_PROTOCOL_UINTTYPES_H_INCLUDED

#include <mtchain/basics/UnorderedContainers.h>
#include <mtchain/basics/base_uint.h>
#include <mtchain/protocol/AccountID.h>

namespace mtchain {
namespace detail {

class CurrencyTag {};
class DirectoryTag {};
class NodeIDTag {};

} // detail

/** Directory is an index into the directory of offer books.
    The last 64 bits of this are the quality. */
using Directory = base_uint<256, detail::DirectoryTag>;

/** Currency is a hash representing a specific currency. */
using Currency = base_uint<160, detail::CurrencyTag>;

/** NodeID is a 160-bit hash representing one node. */
using NodeID = base_uint<160, detail::NodeIDTag>;

/** M currency. */
Currency const& mCurrency();

/** A placeholder for empty currencies. */
Currency const& noCurrency();

/** We deliberately disallow the currency that looks like "M" because too
    many people were using it instead of the correct M currency. */
Currency const& badCurrency();

inline bool isM(Currency const& c)
{
    return c == zero;
}

/** Returns "", "M", or three letter ISO code. */
std::string to_string(Currency const& c);

/** Tries to convert a string to a Currency, returns true on success. */
bool to_currency(Currency&, std::string const&);

/** Tries to convert a string to a Currency, returns noCurrency() on failure. */
Currency to_currency(std::string const&);

inline std::ostream& operator<< (std::ostream& os, Currency const& x)
{
    os << to_string (x);
    return os;
}

} //

namespace std {

template <>
struct hash <mtchain::Currency> : mtchain::Currency::hasher
{
};

template <>
struct hash <mtchain::NodeID> : mtchain::NodeID::hasher
{
};

template <>
struct hash <mtchain::Directory> : mtchain::Directory::hasher
{
};

} // std

#endif
