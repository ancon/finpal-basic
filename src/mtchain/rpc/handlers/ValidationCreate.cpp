//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/Log.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

static
boost::optional<Seed>
validationSeed (Json::Value const& params)
{
    if (!params.isMember (jss::secret))
        return randomSeed ();

    return parseGenericSeed (params[jss::secret].asString ());
}

// {
//   secret: <string>   // optional
// }
//
// This command requires Role::ADMIN access because it makes
// no sense to ask an untrusted server for this.
Json::Value doValidationCreate (RPC::Context& context)
{
    Json::Value     obj (Json::objectValue);

    auto seed = validationSeed(context.params);

    if (!seed)
        return rpcError (rpcBAD_SEED);

    auto const private_key = generateSecretKey (KeyType::secp256k1, *seed);

    obj[jss::validation_public_key] = toBase58 (
        TokenType::TOKEN_NODE_PUBLIC,
        derivePublicKey (KeyType::secp256k1, private_key));

    obj[jss::validation_private_key] = toBase58 (
        TokenType::TOKEN_NODE_PRIVATE, private_key);

    obj[jss::validation_seed] = toBase58 (*seed);
    obj[jss::validation_key] = seedAs1751 (*seed);

    return obj;
}

} //
