//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_JSONRPCUTIL_H_INCLUDED
#define MTCHAIN_SERVER_JSONRPCUTIL_H_INCLUDED

#include <mtchain/json/json_value.h>
#include <mtchain/json/Output.h>

namespace mtchain {

std::string getHTTPHeaderTimestamp ();
void HTTPReply (
    int nStatus, std::string const& strMsg, Json::Output const&, beast::Journal j);

} //

#endif
