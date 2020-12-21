//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_LEGACYPATHFIND_H_INCLUDED
#define MTCHAIN_RPC_LEGACYPATHFIND_H_INCLUDED

#include <atomic>

namespace mtchain {

class Application;

namespace RPC {

class LegacyPathFind
{
public:
    LegacyPathFind (bool isAdmin, Application& app);
    ~LegacyPathFind ();

    bool isOk () const
    {
        return m_isOk;
    }

private:
    static std::atomic <int> inProgress;

    bool m_isOk;
};

} // RPC
} //

#endif
