//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_HANDLER_H_INCLUDED
#define MTCHAIN_RPC_HANDLER_H_INCLUDED

#include <mtchain/core/Config.h>
#include <mtchain/rpc/RPCHandler.h>
#include <mtchain/rpc/Status.h>

namespace Json {
class Object;
}

namespace mtchain {
namespace RPC {

// Under what condition can we call this RPC?
enum Condition {
    NO_CONDITION     = 0,
    NEEDS_NETWORK_CONNECTION  = 1,
    NEEDS_CURRENT_LEDGER  = 2 + NEEDS_NETWORK_CONNECTION,
    NEEDS_CLOSED_LEDGER   = 4 + NEEDS_NETWORK_CONNECTION,
};

struct Handler
{
    template <class JsonValue>
    using Method = std::function <Status (Context&, JsonValue&)>;

    const char* name_;
    Method<Json::Value> valueMethod_;
    Role role_;
    RPC::Condition condition_;
    Method<Json::Object> objectMethod_;
};

const Handler* getHandler (std::string const&);

/** Return a Json::objectValue with a single entry. */
template <class Value>
Json::Value makeObjectValue (
    Value const& value, Json::StaticString const& field = jss::message)
{
    Json::Value result (Json::objectValue);
    result[field] = value;
    return result;
}

} // RPC
} //

#endif
