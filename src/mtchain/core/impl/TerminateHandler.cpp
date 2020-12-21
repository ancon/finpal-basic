//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2012 - 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/core/TerminateHandler.h>
#include <mtchain/basics/Log.h>
#include <mtchain/beast/core/CurrentThreadName.h>

#include <boost/coroutine/exceptions.hpp>
#include <exception>
#include <iostream>
#include <typeinfo>

namespace mtchain {

void terminateHandler()
{
    if (std::current_exception())
    {
        auto const thName =
            beast::getCurrentThreadName().value_or("Unknown");
        try
        {
            throw;
        }
        catch (const std::exception& e)
        {
            auto exName = typeid (e).name();
            std::cerr
                << "Terminating thread " << thName << ": unhandled "
                << exName << " '" << e.what () << "'\n";
            JLOG(debugLog().fatal())
                << "Terminating thread " << thName << ": unhandled "
                << exName << " '" << e.what () << "'\n";
        }
        catch (boost::coroutines::detail::forced_unwind const&)
        {
            std::cerr
                << "Terminating thread " << thName << ": forced_unwind\n";
            JLOG(debugLog().fatal())
                << "Terminating thread " << thName << ": forced_unwind\n";
        }
        catch (...)
        {
            std::cerr
                << "Terminating thread " << thName << ": unknown exception\n";
            JLOG (debugLog().fatal())
                << "Terminating thread " << thName << ": unknown exception\n";
        }
    }
}

}
