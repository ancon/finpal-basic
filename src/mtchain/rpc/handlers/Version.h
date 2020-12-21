//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_MTCHAIN_RPC_HANDLERS_VERSION_H
#define MTCHAIN_MTCHAIN_RPC_HANDLERS_VERSION_H

#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {
namespace RPC {

class VersionHandler
{
public:
    explicit VersionHandler (Context&) {}

    Status check()
    {
        return Status::OK;
    }

    template <class Object>
    void writeResult (Object& obj)
    {
        setVersion (obj);
    }

    static const char* const name()
    {
        return "version";
    }

    static Role role()
    {
        return Role::USER;
    }

    static Condition condition()
    {
        return NO_CONDITION;
    }
};

} // RPC
} //

#endif
