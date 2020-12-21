//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/Log.h>
#include <mtchain/rpc/impl/Handler.h>

namespace mtchain {

Json::Value doLogRotate (RPC::Context& context)
{
    return RPC::makeObjectValue (context.app.logs().rotate());
}

} //
