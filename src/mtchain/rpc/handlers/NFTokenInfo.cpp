//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/rpc/Context.h>
namespace mtchain {

Json::Value doTokenInfo (RPC::Context& context)
{
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    boost::optional<Keylet> k;
    auto const& params = context.params;
    if (params.isMember(sfTokenID.getJsonName()))
    {
        uint256 tokenid;
        if (!tokenid.SetHex(params[sfTokenID.getJsonName()].asString()))
            return rpcError(rpcINVALID_PARAMS);

        k = keylet::nftoken(tokenid);
    }
    else if (params.isMember(sfAssetID.getJsonName()))
    {
        uint256 assetid;
        if (!assetid.SetHex(params[sfAssetID.getJsonName()].asString()))
        {
        }
        else if (params.isMember(jss::id))
        {
            k = keylet::nftoken(assetid, to_blob(params[jss::id].asString()));
        }
        else if (params.isMember(jss::index))
        {
            if (params.isMember(sfOwner.getJsonName()))
            {
                auto owner = parseBase58<AccountID>(params[sfOwner.getJsonName()].asString());
                if (owner)
                {
                    k = keylet::nftoken(assetid, *owner, params[jss::index].asUInt());
                    auto sleOwnerTokenIndex = ledger->read(*k);
                    if (sleOwnerTokenIndex)
                    {
                        k = keylet::nftoken(sleOwnerTokenIndex->getFieldH256(sfTokenID));
                    }
                }
            }
            else
            {
                k = keylet::nftoken(assetid, params[jss::index].asUInt());
                auto sleTokenIndex = ledger->read(*k);
                if (sleTokenIndex)
                {
                    k = keylet::nftoken(sleTokenIndex->getFieldH256(sfTokenID));
                }
            }
        }
    }

    if (!k)
    {
        return rpcError(rpcINVALID_PARAMS);
    }

    auto sleToken = ledger->read(*k);
    if (!sleToken)
    {
        return rpcError(rpcNO_TOKEN);
    }

    result[jss::info] = sleToken->getJson(0);
    return result;
}

Json::Value doAccountAllTokenInfo(RPC::Context& context)
{
    auto const& params(context.params);
    if(!params.isMember(jss::Account))
       return RPC::missing_field_error(jss::Account);
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger(ledger,context);
    if(!ledger)   return result;
    auto account = parseBase58<AccountID>(params[jss::Account].asString());
    if(!account)  return rpcError(rpcINVALID_PARAMS);
    if(!ledger->exists(keylet::account(*account))) return rpcError(rpcNO_ACCOUNT);
   
    int limit = params.isMember(jss::limit)?params[jss::limit].asUInt():10;

    // get the num of account's tokens
    auto sle = ledger->read(keylet::account(*account));
    auto assetNum = sle->getFieldU64(sfAssetNumber);

    //get account's asset info
    Json::Value info = Json::objectValue;
    Json::Value& jsonTokens(info[jss::tokens] = Json::arrayValue);
    Json::Value marker = Json::objectValue;
    int i,j;
    if(!params.isMember(jss::marker))
    {
	i = 0;
	j = 0;
    }
    else
    {
    	marker = params[jss::marker]; 
	i = marker["assetIndex"].asInt();
	j = marker["tokenIndex"].asInt();
	if(marker["assetIndex"].asInt() < 0 || marker["tokenIndex"].asInt() < 0)
	{
		return rpcError(rpcINVALID_PARAMS);
	}
    }
    int count = 0;
    for(; i < assetNum+1 && count < limit+1; i++)
    {
		auto sleAsset = doGetAssetByAccount(*ledger,*account,i);
		if(!sleAsset)
		{
			result[jss::info] = info;
			return result;
		}
		auto tokenNum = sleAsset->getFieldU64(sfTokenNumber);
		uint256 assetid =sleAsset->getFieldH256(sfAssetID);
		for(; (j < tokenNum) && (count < limit+1) ; j++)	
		{
			if(!doGetTokenByAssetId(*ledger,assetid,j,*account) )  //when do not have data
			{
				if(j == marker[jss::tokenIndex].asInt())	// when j == marker[jss::tokenIndex].asInt(), return rpcERROR
				{
					return rpcError (rpcNO_TOKEN);
				}
				else	// when count == limit or other conditions, set marker = null.
				{
					result[jss::info] = info;
					return result;
				}
			}
			else if(count < limit)
			{
				jsonTokens.append(doGetTokenByAssetId(*ledger,assetid,j,*account)->getJson(0));
			}
			else
			{
    				marker[jss::assetIndex] = i;
				marker[jss::tokenIndex] = j;
				result[jss::marker] = marker;
    				result[jss::info] = info;
				return result;
			}
			count++;
		}
    }

    return result;
}


} //
