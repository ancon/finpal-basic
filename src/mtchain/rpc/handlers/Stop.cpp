//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/json/json_value.h>
#include <mtchain/rpc/impl/Handler.h>
#include <mtchain/basics/make_lock.h>

namespace mtchain {

namespace RPC {
struct Context;
}

Json::Value doStop (RPC::Context& context)
{
    auto lock = make_lock(context.app.getMasterMutex());
    context.app.signalStop ();

    return RPC::makeObjectValue (systemName () + " server stopping");
}

} //
