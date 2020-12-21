//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_OUTPUT_H_INCLUDED
#define MTCHAIN_RPC_OUTPUT_H_INCLUDED

#include <boost/utility/string_ref.hpp>

namespace mtchain {
namespace RPC {

using Output = std::function <void (boost::string_ref const&)>;

inline
Output stringOutput (std::string& s)
{
    return [&](boost::string_ref const& b) { s.append (b.data(), b.size()); };
}

} // RPC
} //

#endif
