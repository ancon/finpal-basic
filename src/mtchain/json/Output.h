//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_JSON_OUTPUT_H_INCLUDED
#define MTCHAIN_JSON_OUTPUT_H_INCLUDED

#include <boost/utility/string_ref.hpp>
#include <functional>

namespace Json {

class Value;

using Output = std::function <void (boost::string_ref const&)>;

inline
Output stringOutput (std::string& s)
{
    return [&](boost::string_ref const& b) { s.append (b.data(), b.size()); };
}

/** Writes a minimal representation of a Json value to an Output in O(n) time.

    Data is streamed right to the output, so only a marginal amount of memory is
    used.  This can be very important for a very large Json::Value.
 */
void outputJson (Json::Value const&, Output const&);

/** Return the minimal string representation of a Json::Value in O(n) time.

    This requires a memory allocation for the full size of the output.
    If possible, use outputJson().
 */
std::string jsonAsString (Json::Value const&);

} // Json

#endif
