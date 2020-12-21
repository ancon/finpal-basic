//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/json/JsonPropertyStream.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doPrint (RPC::Context& context)
{
    JsonPropertyStream stream;
    if (context.params.isObject()
        && context.params[jss::params].isArray()
        && context.params[jss::params][0u].isString ())
    {
        context.app.write (stream, context.params[jss::params][0u].asString());
    }
    else
    {
        context.app.write (stream);
    }

    return stream.top();
}

} //
