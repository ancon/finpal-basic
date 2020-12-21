//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_HTTPCLIENT_H_INCLUDED
#define MTCHAIN_TEST_HTTPCLIENT_H_INCLUDED

#include <test/jtx/AbstractClient.h>
#include <mtchain/core/Config.h>
#include <memory>

namespace mtchain {
namespace test {

/** Returns a client using JSON-RPC over HTTP/S. */
std::unique_ptr<AbstractClient>
makeJSONRPCClient(Config const& cfg, unsigned rpc_version = 2);

} // test
} //

#endif
