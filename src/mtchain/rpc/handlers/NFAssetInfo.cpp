//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doAssetBalance (RPC::Context& context)
{
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    auto const& params = context.params;
    if (!params.isMember(jss::Account) || !params.isMember(sfAssetID.getJsonName()))
        return rpcError(rpcINVALID_PARAMS);

    auto account = parseBase58<AccountID>(params[jss::Account].asString());
    if (!account)
        return rpcError(rpcINVALID_PARAMS);

    uint256 assetid;
    if (!assetid.SetHex(params[sfAssetID.getJsonName()].asString()))
        return rpcError(rpcINVALID_PARAMS);

    if (!ledger->read(keylet::account(*account)))
        return rpcError (rpcNO_ACCOUNT);

    if (!ledger->read(keylet::nfasset(assetid)))
        return rpcError (rpcNO_ASSET);

    auto sleAssetOwner = ledger->read(keylet::nfasset(assetid, *account));
    std::uint64_t ownerTokenNum = sleAssetOwner ? sleAssetOwner->getFieldU64(sfTokenNumber) : 0;

    result[jss::Account] = params[jss::Account];
    result[sfAssetID.getJsonName()] = params[sfAssetID.getJsonName()];
    result[jss::Amount] = to_string(ownerTokenNum);

    return result;
}


Json::Value doAssetInfo (RPC::Context& context)
{
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    uint256 assetid;
    auto const& params = context.params;
    if (params.isMember(sfAssetID.getJsonName()))
    {
        if (!assetid.SetHex(params[sfAssetID.getJsonName()].asString()))
            return rpcError(rpcINVALID_PARAMS);
    }
    else if (params.isMember(jss::Account))
    {
        auto account = parseBase58<AccountID>(params[jss::Account].asString());
        if (!account) return rpcError(rpcINVALID_PARAMS);

        std::uint64_t assetIndex = params.isMember(jss::index) ? params[jss::index].asUInt() : 0;
        auto const& k = keylet::nfasset(*account, assetIndex);
        auto sleAssetIndex = ledger->read(k);
        if (!sleAssetIndex) return rpcError(rpcNO_ASSET);
        assetid = sleAssetIndex->getFieldH256(sfAssetID);
    }
    else
    {
        return rpcError (rpcINVALID_PARAMS);
    }

    auto sleAsset = ledger->read(keylet::nfasset(assetid));
    if (!sleAsset)
    {
        return rpcError (rpcNO_ASSET);
    }

    result[jss::info] = sleAsset->getJson(0);
    return result;
}


Json::Value doIsAssetOperator(RPC::Context& context)
{
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    auto const& params = context.params;
    if (!params.isMember(jss::Account) || !params.isMember(sfAssetID.getJsonName()) ||
        !params.isMember(sfOwner.getJsonName()))
        return rpcError (rpcINVALID_PARAMS);

    auto account = parseBase58<AccountID>(params[jss::Account].asString());
    if (!account)
        return rpcError(rpcINVALID_PARAMS);

    uint256 assetid;
    if (!assetid.SetHex(params[sfAssetID.getJsonName()].asString()))
        return rpcError(rpcINVALID_PARAMS);

    auto owner = parseBase58<AccountID>(params[sfOwner.getJsonName()].asString());
    if (!owner)
        return rpcError(rpcINVALID_PARAMS);

    if (*account == *owner)
        return rpcError(rpcINVALID_PARAMS);

    if (!ledger->read(keylet::account(*account)))
        return rpcError (rpcNO_ACCOUNT);

    if (!ledger->read(keylet::nfasset(assetid)))
        return rpcError (rpcNO_ASSET);

    if (!ledger->read(keylet::account(*owner)))
        return rpcError (rpcNO_OWNER);

    auto const& k = keylet::nfasset(assetid, *owner, *account);

    result[jss::Account] = params[jss::Account];
    result[sfAssetID.getJsonName()] = params[sfAssetID.getJsonName()];
    result[sfOwner.getJsonName()] = params[sfOwner.getJsonName()];
    result["isOperator"] = ledger->exists(k);
    return result;
}

std::shared_ptr<SLE const> doGetAssetByAccount(ReadView const& view, AccountID const& id, int assetIndex)
{
	auto const& k = keylet::nfasset(id, assetIndex);
        auto sleAssetIndex = view.read(k);
    	uint256 assetid;
        if(sleAssetIndex)
        {
            assetid = sleAssetIndex->getFieldH256(sfAssetID);
        }
        auto sleAsset = view.read(keylet::nfasset(assetid));
	if(!sleAsset)
	{
		return nullptr;
	}
	return sleAsset;
}

Json::Value doAccountAllAssetInfo(RPC::Context& context)
{
    auto const& params(context.params);
    if(!params.isMember(jss::Account))
	return RPC::missing_field_error(jss::Account);
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger(ledger,context);
    if(!ledger)   return result;
    auto account = parseBase58<AccountID>(params[jss::Account].asString());
    if(!account)  return rpcError(rpcINVALID_PARAMS);
    if(!ledger->exists(keylet::account(*account)))   return rpcError(rpcNO_ACCOUNT);
   
    unsigned int limit = params.isMember(jss::limit)?params[jss::limit].asUInt():10;
    int marker = params.isMember(jss::marker)?params[jss::marker].asUInt():0;

    // get the num of account's asset
    auto sle = ledger->read(keylet::account(*account));
    auto assetNum = sle->getFieldU64(sfAssetNumber);

    //get account's asset info
    Json::Value info = Json::objectValue;
    Json::Value& jsonAssets(info[jss::assets] = Json::arrayValue);
    int resultsEndIndex, count = 0;
    resultsEndIndex = ((assetNum - marker) > limit)? (limit+marker):assetNum;
    int i;
    for(i = marker; (i < (resultsEndIndex+1)) &&(count < assetNum);i++)
    {
	if(!doGetAssetByAccount(*ledger, *account, i))
	{
		if(i != marker)
		{
			result[jss::info] = info;
			return result;
		}
		else
		{
			return rpcError(rpcNO_ASSET);
		}
	}
	else if(i == resultsEndIndex)
	{
		result[jss::marker] = std::to_string(i);
	}
	else
	{
		jsonAssets.append(doGetAssetByAccount(*ledger,*account,i)->getJson(0));
		count++;
	}
    }
    result[jss::info] = info; 
    return result;
}
std::shared_ptr<SLE const> doGetTokenByAssetId(ReadView const& view, uint256 assetid, int tokenIndex, AccountID const& id)
{
	auto const& k = (!id)?(keylet::nftoken(assetid,tokenIndex)):(keylet::nftoken(assetid,id,tokenIndex));
        auto sleAssetIndex = view.read(k);
    	uint256 tokenid;
        if(sleAssetIndex)
        {
            tokenid = sleAssetIndex->getFieldH256(sfTokenID);
        }
        auto sleToken = view.read(keylet::nftoken(tokenid));
	if(!sleToken)
	{
		return nullptr;
	}
	return sleToken;
}
Json::Value doAssetAllTokenInfo(RPC::Context& context)
{
       // look up ledger
       std::shared_ptr<ReadView const> ledger;
       auto result = RPC::lookupLedger(ledger,context);
       if(!ledger)       return result;
       auto const& params = context.params;
       uint256 assetid;
       
       // verify the accout and asset is valid
       if (!params.isMember(sfAssetID.getJsonName()) ||
	     !assetid.SetHex(params[sfAssetID.getJsonName()].asString()))
       	       return rpcError(rpcINVALID_PARAMS);
       
	int limit = params.isMember(jss::limit)?params[jss::limit].asUInt():10;
	int marker = params.isMember(jss::marker)?params[jss::marker].asUInt():0;

	if(limit < 0 || marker < 0)
	{
		return rpcError(rpcINVALID_PARAMS);
	}
	auto sleAsset = ledger->read(keylet::nfasset(assetid));
	if(!sleAsset)	return rpcError(rpcNO_ASSET);

       //get the number of asset's tokens
       auto tokenNum = sleAsset->getFieldU64(sfTokenNumber);

       // get asset's token info
       boost::optional<Keylet> k;
       Json::Value info = Json::objectValue;
       Json::Value& jsonTokens(info[jss::tokens] = Json::arrayValue);
       int resultsEndIndex;
       resultsEndIndex = ((tokenNum - marker) > limit)?(marker+limit):tokenNum;
	AccountID account;
	int i;
       for(i = marker;i < resultsEndIndex+1 ;i++)
       {
		if(!doGetTokenByAssetId(*ledger,assetid,i,account))
		{
			if(i == marker)
			{
				return rpcError(rpcNO_TOKEN);
			}
		}
		else if(i == resultsEndIndex)
		{
			result[jss::marker] = std::to_string(i);
		}
		else 
		{
	        	jsonTokens.append(doGetTokenByAssetId(*ledger,assetid,i,account)->getJson(0));
		}
       }
       result[jss::info] = info;
       return result;  
}


} //
