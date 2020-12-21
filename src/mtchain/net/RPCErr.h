//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NET_RPCERR_H_INCLUDED
#define MTCHAIN_NET_RPCERR_H_INCLUDED

#include <mtchain/json/json_value.h>

namespace mtchain {

// VFALCO NOTE these are deprecated
bool isRpcError (Json::Value jvResult);
Json::Value rpcError (int iError,
                      Json::Value jvResult = Json::Value (Json::objectValue));

} //

#endif
