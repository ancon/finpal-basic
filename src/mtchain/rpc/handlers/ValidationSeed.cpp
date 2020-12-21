//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/core/Config.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/basics/make_lock.h>
#include <iostream>

namespace mtchain {

// {
//   secret: <string>
// }
Json::Value doValidationSeed (RPC::Context& context)
{
    // TODO: This feature is temporarily disabled since we
    // cannot modify the config object. We should consider
    // whether we want the ability to change the validator
    // keys at runtime.
    return rpcError (rpcNOT_IMPL);

    // auto lock = make_lock(context.app.getMasterMutex());
    // Json::Value obj (Json::objectValue);

    // if (!context.params.isMember (jss::secret))
    // {
    //     std::cerr << "Unset validation seed." << std::endl;

    //     context.app.config().VALIDATION_SEED.clear ();
    //     context.app.config().VALIDATION_PUB.clear ();
    //     context.app.config().VALIDATION_PRIV.clear ();
    // }
    // else if (!context.app.config().VALIDATION_SEED.setSeedGeneric (
    //     context.params[jss::secret].asString ()))
    // {
    //     context.app.config().VALIDATION_PUB.clear ();
    //     context.app.config().VALIDATION_PRIV.clear ();

    //     return rpcError (rpcBAD_SEED);
    // }
    // else
    // {
    //     auto& seed = context.app.config().VALIDATION_SEED;
    //     auto& pub = context.app.config().VALIDATION_PUB;

    //     pub = MTChainAddress::createNodePublic (seed);
    //     context.app.config().VALIDATION_PRIV = MTChainAddress::createNodePrivate (seed);

    //     obj[jss::validation_public_key] = pub.humanNodePublic ();
    //     obj[jss::validation_seed] = seed.humanSeed ();
    //     obj[jss::validation_key] = seed.humanSeed1751 ();
    // }

    // return obj;
}

} //
