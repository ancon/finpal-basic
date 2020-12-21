//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/json_value.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

// {
//   secret: <string>
// }
Json::Value doWalletSeed (RPC::Context& context)
{
    boost::optional<Seed> seed;

    bool bSecret = context.params.isMember (jss::secret);

    if (bSecret)
        seed = parseGenericSeed (context.params[jss::secret].asString ());
    else
        seed = randomSeed ();

    if (!seed)
        return rpcError (rpcBAD_SEED);

    Json::Value obj (Json::objectValue);
    obj[jss::seed]     = toBase58(*seed);
    obj[jss::key]      = seedAs1751(*seed);
    obj[jss::deprecated] = "Use wallet_propose instead";
    return obj;
}

} //
