//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Role.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

namespace RPC {
struct Context;
} // RPC

Json::Value doPing (RPC::Context& context)
{
    // For testing connection privileges.
    if (isUnlimited(context.role))
    {
        Json::Value ret;

        switch (context.role)
        {
            case Role::ADMIN:
                ret[jss::role] = "admin";
                break;
            case Role::IDENTIFIED:
                ret[jss::role] = "identified";
                break;
            default:
                ;
        }

        return ret;
    }
    else
    {
        return Json::Value (Json::objectValue);
    }
}

} //
