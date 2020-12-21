//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>
#include <cstdlib>
#include <exception>
#include <iostream>

namespace mtchain {

namespace detail {

[[noreturn]]
void
accessViolation() noexcept
{
    // dereference memory location zero
    int volatile* j = 0;
    (void)*j;
    std::abort ();
}

} // detail

void
LogThrow (std::string const& title)
{
    JLOG(debugLog().warn()) << title;
}

[[noreturn]]
void
LogicError (std::string const& s) noexcept
{
    JLOG(debugLog().fatal()) << s;
    std::cerr << "Logic error: " << s << std::endl;
    detail::accessViolation();
}

} //
