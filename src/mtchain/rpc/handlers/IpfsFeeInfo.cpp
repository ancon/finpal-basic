#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/crypto/KeyType.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <ed25519-donna/ed25519.h>
#include <boost/optional.hpp>
#include <cmath>
#include <map>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/app/main/Application.h>
#include <BeastConfig.h> 
 
namespace mtchain {
    extern Application& getApp();
 
    Json::Value doIpfsFeeInfo (RPC::Context& context)
    {
        auto &app = mtchain::getApp();
	
        std::string accountID;
        //boost::optional<std::uint32_t> storageFee,downloadFee;
        int storageFee,downloadFee;
        Json::Value ret;
    Section const& section = app.config().section("IPFS");
    set(accountID,"account_id",section);
    set(storageFee,"storage_fee",section);
    set(downloadFee,"download_fee",section);
    if(accountID.size() != 0 && storageFee!= 0 && downloadFee != 0)
    {
        ret[jss::accountID] = accountID;
        ret[jss::storageFee] = storageFee;
        ret[jss::downloadFee] = downloadFee;
    }
    else{
            ret[jss::status] = "false";
    }
        return ret;
    }
}
