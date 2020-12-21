//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/PayChan.h>
#include <mtchain/protocol/STAccount.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/impl/Tuning.h>

namespace mtchain {

// {
//   secret_key: <signing_secret_key>
//   channel_id: 256-bit channel id
//   drops: 64-bit uint (as string)
// }
Json::Value doChannelAuthorize (RPC::Context& context)
{
    auto const& params (context.params);
    for (auto const& p : {jss::secret, jss::channel_id, jss::amount})
        if (!params.isMember (p))
            return RPC::missing_field_error (p);

    Json::Value result;
    auto const keypair = RPC::keypairForSignature (params, result);
    if (RPC::contains_error (result))
        return result;

    uint256 channelId;
    if (!channelId.SetHexExact (params[jss::channel_id].asString ()))
        return rpcError (rpcCHANNEL_MALFORMED);

    std::uint64_t drops = 0;
    try
    {
        drops = std::stoul (params[jss::amount].asString ());
    }
    catch (std::exception const&)
    {
        return rpcError (rpcCHANNEL_AMT_MALFORMED);
    }

    Serializer msg;
    serializePayChanAuthorization (msg, channelId, MAmount (drops));

    try
    {
        auto const buf = sign (keypair.first, keypair.second, msg.slice ());
        result[jss::signature] = strHex (buf);
    }
    catch (std::exception&)
    {
        result =
            RPC::make_error (rpcINTERNAL, "Exception occurred during signing.");
    }
    return result;
}

// {
//   public_key: <public_key>
//   channel_id: 256-bit channel id
//   drops: 64-bit uint (as string)
//   signature: signature to verify
// }
Json::Value doChannelVerify (RPC::Context& context)
{
    auto const& params (context.params);
    for (auto const& p :
        {jss::public_key, jss::channel_id, jss::amount, jss::signature})
        if (!params.isMember (p))
            return RPC::missing_field_error (p);

    std::string const strPk = params[jss::public_key].asString ();
    auto const pk =
        parseBase58<PublicKey> (TokenType::TOKEN_ACCOUNT_PUBLIC, strPk);
    if (!pk)
        return rpcError (rpcPUBLIC_MALFORMED);

    uint256 channelId;
    if (!channelId.SetHexExact (params[jss::channel_id].asString ()))
        return rpcError (rpcCHANNEL_MALFORMED);

    std::uint64_t drops = 0;
    try
    {
        drops = std::stoul (params[jss::amount].asString ());
    }
    catch (std::exception const&)
    {
        return rpcError (rpcCHANNEL_AMT_MALFORMED);
    }

    std::pair<Blob, bool> sig(strUnHex (params[jss::signature].asString ()));
    if (!sig.second || !sig.first.size ())
        return rpcError (rpcINVALID_PARAMS);

    Serializer msg;
    serializePayChanAuthorization (msg, channelId, MAmount (drops));

    Json::Value result;
    result[jss::signature_verified] =
        verify (*pk, msg.slice (), makeSlice (sig.first), /*canonical*/ true);
    return result;
}

} //
