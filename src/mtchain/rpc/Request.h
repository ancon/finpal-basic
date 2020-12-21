//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_REQUEST_H_INCLUDED
#define MTCHAIN_RPC_REQUEST_H_INCLUDED

#include <mtchain/resource/Charge.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/json/json_value.h>
#include <beast/utility/Journal.h>

namespace mtchain {

class Application;

namespace RPC {

struct Request
{
    explicit Request (
        beast::Journal journal_,
        std::string const& method_,
        Json::Value& params_,
        Application& app_)
        : journal (journal_)
        , method (method_)
        , params (params_)
        , fee (Resource::feeReferenceRPC)
        , app (app_)
    {
    }

    // [in] The Journal for logging
    beast::Journal journal;

    // [in] The JSON-RPC method
    std::string method;

    // [in] The MTChain-specific "params" object
    Json::Value params;

    // [in, out] The resource cost for the command
    Resource::Charge fee;

    // [out] The JSON-RPC response
    Json::Value result;

    // [in] The Application instance
    Application& app;

private:
    Request& operator= (Request const&);
};

}
}

#endif
