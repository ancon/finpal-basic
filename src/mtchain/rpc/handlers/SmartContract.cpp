//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.
*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/crypto/KeyType.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/handlers/SmartContract.h>
#include <ed25519-donna/ed25519.h>
#include <boost/optional.hpp>
#include <cmath>
#include <map>
#include <mtchain/basics/StringUtilities.h>
#include <array>
#include <atomic>
#include <mtchain/app/main/Application.h>
#include <mtchain/resource/Fees.h>

#include "../../rpc/handlers/WalletPropose.h"
#include "../../rpc/Context.h"
#include "../../rpc/handlers/Handlers.h"
#include "../../rpc/handlers/SmartContract.h"
#include <mtchain/net/RPCCall.h>
#include "../../../test/jtx/Env.h"

#include <mtchain/json/json_reader.h>



//using namespace mtchain;
std::map<lua_State*, mtchain::RPC::Context*> g_map_context_lvm;

typedef struct context_lvm
{
	lua_State* L;
	mtchain::RPC::Context* pContext;
}context_lvm;

std::map<std::string, context_lvm* > g_map_hash_context_lvm;


std::mutex m_;
std::condition_variable cv_;


//////////////////////////////////////////////////////////////////////////
typedef mtchain::RPC::Context* luaContext;
typedef std::string*  luaResult;

template <size_t size>
class SmartContractCounters

{
public:
    SmartContractCounters()
    {
        for (int i = 0; i < size; ++i)
        {
		atomic_init(&counters[i], 0UL);
        }
    }

    size_t count(int stage)
    {
        size_t i;
        if (stage) // current smart contract finsihes
        {
            i = --counters[0]; // 0 used to the number of current smart contract
	}
	else // start a new smart contract
	{
            i = ++counters[0];
            ++counters[std::min(i, counters.size() - 1)];
        }

	return i;
    }

    size_t getCounterHistory(size_t cnt)
    {
        return counters[std::min(cnt, counters.size() - 1)];
    }

private:
    std::array<std::atomic<size_t>, size> counters;
};

SmartContractCounters<11> scCounters;

#if 0
typedef struct NumArray {
	int size; //表示数组的大小
	double values[1]; //此处的values仅代表一个double*类型的指针，values指向NumArray结构后部紧跟的数据的地址
} NumArray;

static int newarray(lua_State *L) {
	int n = luaL_checkint(L, 1);
	size_t nbytes = sizeof(NumArray) + (n - 1) * sizeof(double);
	NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);
	a->size = n;
	return 1;  /* new userdatum is already on the stack */
}


static int setarray(lua_State *L) {
	NumArray *a = (NumArray *)lua_touserdata(L, 1);
	int index = luaL_checkint(L, 2);
	double value = luaL_checknumber(L, 3);

	luaL_argcheck(L, a != NULL, 1, "`array' expected");

	luaL_argcheck(L, 1 <= index && index <= a->size, 2,
		"index out of range");

	a->values[index - 1] = value;
	return 0;
}


static int getarray(lua_State *L) {
	NumArray *a = (NumArray *)lua_touserdata(L, 1);
	int index = luaL_checkint(L, 2);

	luaL_argcheck(L, a != NULL, 1, "`array' expected");

	luaL_argcheck(L, 1 <= index && index <= a->size, 2,
		"index out of range");

	lua_pushnumber(L, a->values[index - 1]);
	return 1;
}
//////////////////////////////////////////////////////////////////////////
#endif

static int average(lua_State *L)
{
	/* get number of arguments */
	int n = lua_gettop(L);
	double sum = 0;
	int i;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{
		/* total the arguments */
		sum += lua_tonumber(L, i);
	}

	/* push the average */
	lua_pushnumber(L, sum / n);

	/* push the sum */
	lua_pushnumber(L, sum);

	// call wallet propose
	//Json::Value j = mtchain::walletPropose(NULL);
	//std::cout << j.toStyledString();

	// test payment
	//mtchain::test::Env_SmartContract_test* pEsc = new mtchain::test::Env_SmartContract_test();
	//pEsc->testEnv();

	/* return the number of results */

	{
		std::chrono::milliseconds const timeout(5000);
		std::unique_lock<std::mutex> lock(m_);
		if (!cv_.wait_for(lock, timeout,
			[&]
		{
			for (auto it = g_map_context_lvm.begin();
				it != g_map_context_lvm.end(); ++it)
			{
				if (it->first == L)
				{
					//m = std::move(*it);
					//msgs_.erase(it);
					//mtchain::RPC::Context* context = it->second;
					
					return true;
				}
			}
			return false;
		}))
		{
			return 0;
		}
	}
	return 2;
}

static mtchain::RPC::Context& getSmartContractContext(lua_State *L)
{
	int n = lua_gettop(L);
	lua_pushstring(L, "Context");
	lua_gettable(L, LUA_REGISTRYINDEX);
	mtchain::RPC::Context& context = *(mtchain::RPC::Context*)lua_topointer(L, -1);
	lua_pop(L, 1);
	(void)n;
	BOOST_ASSERT(lua_gettop(L) == n);

	return context;
}

static std::string &getSmartContractResult(lua_State *L)
{
	int n = lua_gettop(L);
	lua_pushstring(L, "Result");
	lua_gettable(L, LUA_REGISTRYINDEX);
	std::string& result = *(std::string*)lua_topointer(L, -1);
	lua_pop(L, 1);
	(void)n;
	BOOST_ASSERT(lua_gettop(L) == n);

	return result;
}

static int scWalletPropose(lua_State *L)
{
	// call walletPropose
	Json::Value params;
	Json::Value j = mtchain::walletPropose(params);
	std::cout << j.toStyledString();


	int n = lua_gettop(L);
	int i = 0;
	std::string param = "";

	for (i = 1; i <= n; i++)
	{
		/* total the arguments */
		param = lua_tostring(L, i);
		std::cout << param << std::endl;
	}

	// push the result string
	if (lua_pushstring(L, j.toStyledString().c_str()))
	{
		return 1;
	}

	/* return error */
	return -1;
}

/*
	 transfer function for smart contract
*/
std::string scTransfer_test(mtchain::RPC::Context& context)
{
// 	auto config = std::make_unique<mtchain::Config>();
// 	config->setup("", false, false, false);
// 
// 	beast::severities::Severity thresh = beast::severities::Severity::kInfo;
// 	auto logs = std::make_unique<mtchain::Logs>(thresh);

	Json::Value j;
	context.params[mtchain::jss::command] = "submit";
	context.params[mtchain::jss::secret] = "snoPB6XtMeMyMHUVTgbuqAfg1SUTb";

	Json::Value     txJSON;
	Json::Reader    reader;
	std::string  tx_str = "{\"TransactionType\":\"Payment\",\"Account\":\"6Hb9CJAWyB46j91VRWn9rDkukG4bwdtyTh\",\"Amount\":\"100000000\",\"Destination\":\"6nUy2SHT6B9DubsPmkJZUXTf5FcNDG6YEA\"}";
	reader.parse(tx_str.c_str(), txJSON);
	context.params[mtchain::jss::tx_json] = txJSON;

	mtchain::RPC::doCommand(context, j);
	JLOG(context.j.debug()) << "result: " << j.toStyledString();

	std::string sc = j.toStyledString();
	return sc;
}

/*
transfer function for smart contract
*/
#if 0
static int scTransfer2(mtchain::RPC::Context& context, std::string str_secret, std::string str_tx_json)
{
	Json::Value j;
	context.params[mtchain::jss::command] = "submit";
	context.params[mtchain::jss::secret] = str_secret;

	Json::Value     txJSON;
	Json::Reader    reader;

	reader.parse(str_tx_json.c_str(), txJSON);
	context.params[mtchain::jss::tx_json] = txJSON;

	mtchain::RPC::doCommand(context, j);
	JLOG(context.j.debug()) << "result: " << j.toStyledString();


	return 1;
}
#endif

/**
	transfer function for smart contract
*/
static int  scTransfer(lua_State *L)
{
	/* get number of arguments */
	if (lua_gettop(L) < 2)
	{
		return -1;
	}

	std::string str_secret = lua_tostring(L, 1u);
	std::string str_tx_json = lua_tostring(L, 2u);
    
	mtchain::RPC::Context& context = getSmartContractContext(L);
// 	auto pContext = *(luaContext**)lua_touserdata(L, -2);
// 	mtchain::RPC::Context& context = (mtchain::RPC::Context&)*pContext;

	Json::Value j;
	context.params[mtchain::jss::command] = "submit";
	context.params[mtchain::jss::secret] = str_secret;

	Json::Value     txJSON;
	Json::Reader    reader;

	reader.parse(str_tx_json.c_str(), txJSON);
	context.params[mtchain::jss::tx_json] = txJSON;

	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scFrozenAsset function for smart contract
*/
static int  scFrozenAsset(lua_State *L)
{
	/* get number of arguments */
	if (lua_gettop(L) < 2)
	{
		return -1;
	}

	std::string str_secret = lua_tostring(L, 1u);
	std::string str_tx_json = lua_tostring(L, 2u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "submit";
	context.params[mtchain::jss::secret] = str_secret;

	Json::Value     txJSON;
	Json::Reader    reader;

	reader.parse(str_tx_json.c_str(), txJSON);
	context.params[mtchain::jss::tx_json] = txJSON;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}


/**
	scAccountChannels function for smart contract
*/
static int  scAccountChannels(lua_State *L)
{
	/* get number of arguments */
	//int n = lua_gettop(L);

	std::string str_account = lua_tostring(L, 1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "account_channels";
	context.params[mtchain::jss::account] = str_account;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scUnfrozenAsset function for smart contract
*/
static int  scUnfrozenAsset(lua_State *L)
{
	/* get number of arguments */
	if (lua_gettop(L) < 2)
	{
		return -1;
	}

	std::string str_secret = lua_tostring(L, 1u);
	std::string str_tx_json = lua_tostring(L, 2u);

	mtchain::RPC::Context& context = getSmartContractContext(L);
	
	context.params[mtchain::jss::command] = "submit";
	context.params[mtchain::jss::secret] = str_secret;

	Json::Value     txJSON;
	Json::Reader    reader;

	reader.parse(str_tx_json.c_str(), txJSON);
	context.params[mtchain::jss::tx_json] = txJSON;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scAccountInfo function for smart contract
*/
static int  scAccountInfo(lua_State *L)
{
	/* get number of arguments */
	if (lua_gettop(L) < 1)
	{
		return -1;
	}

	std::string str_account = lua_tostring(L, 1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);
	
	context.params[mtchain::jss::command] = "account_info";
	context.params[mtchain::jss::account] = str_account;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scAccountLines function for smart contract
*/
static int  scAccountLines(lua_State *L)
{
	/* get number of arguments */
	if (lua_gettop(L) < 1)
	{
		return -1;
	}

	std::string str_account = lua_tostring(L, 1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "account_lines";
	context.params[mtchain::jss::account] = str_account;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scAccountTx function for smart contract
*/
static int  scAccountTx(lua_State *L)
{
	/* get number of arguments */
	//int n = lua_gettop(L);

	std::string str_account = lua_tostring(L, 1u);
	int minLedger = lua_tonumber(L, 2u);
	int maxLedger = lua_tonumber(L, 3u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "account_tx";
	context.params[mtchain::jss::account] = str_account;
	context.params[mtchain::jss::ledger_index_min] = minLedger;
	context.params[mtchain::jss::ledger_index_max] = maxLedger;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scLedgerClosed function for smart contract
*/
static int  scLedgerClosed(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	//context.params[mtchain::jss::command] = "ledger_closed";
	context.params[mtchain::jss::command] = "ledger";
	context.params[mtchain::jss::ledger_index] = "closed";

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
scLedger function for smart contract
commandLine format:
	ledger [<id>|current|closed|validated] [full]
*/
static int scLedger(lua_State *L) {
	if (lua_gettop(L) < 1) {
		return -1;
	}
	std::string str_ledgerIndex = lua_tostring(L, 1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "ledger";
	context.params[mtchain::jss::ledger_index] = str_ledgerIndex;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);
	std::string result = j.toStyledString();
	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scSimLedgerAccept function for smart contract
	commandLine format:
		ledger_accept
*/
static int  scSimLedgerAccept(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "ledger_accept";

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}
/**
	scSimLedgerCurrent function for smart contract
	commandLine format:
		ledger_current
*/
static int  scSimLedgerCurrent(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "ledger_current";

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}


/**
	scSimLedgerRequest function for smart contract
	commandLine format:
		ledger_request
*/
static int  scSimLedgerRequest(lua_State *L)
{
	/*
		get number of arguments
	*/
	if(lua_gettop(L) < 1)	
	{
		return -1;
	}
	std::string ledger_index = lua_tostring(L,1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "ledger_request";
	context.params[mtchain::jss::ledger_index] = ledger_index;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

/**
	scServerInfo function for smart contract
	commandLine format:
		server_info
*/
static int  scServerInfo(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "server_info";

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

int test_sc_dofile()
{
	/* initialize Lua */
	lua_State *L = luaL_newstate();

	/* load Lua base libraries */
	luaL_openlibs(L);

	/* register our function */
	lua_register(L, "average", average);

	/* run the script */
	luaL_dofile(L, "avg.lua");

	/* cleanup Lua */
	lua_close(L);

	return 0;
}

static int init(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	std::string s = context.params[mtchain::jss::secret].asString();
	s = context.params[mtchain::jss::transaction].asString();

	Json::Value tx_json = context.params[mtchain::jss::tx_json];
	
	if (tx_json == Json::Value::null)
	{
		return 0;
	}

	std::string tx_json_str = tx_json.toStyledString();
	lua_pushstring(L, tx_json_str.c_str());

	return 1;
}


static int init_s(lua_State *L)
{
	mtchain::RPC::Context& context = getSmartContractContext(L);

	std::string s = context.params[mtchain::jss::secret].asString();
	std::string tx = context.params[mtchain::jss::transaction].asString();

	Json::Value tx_json = context.params[mtchain::jss::tx_json];
	std::string tx_json_str = tx_json.toStyledString();

	lua_pushstring(L, s.c_str());
	lua_pushstring(L, tx_json_str.c_str());

	return 2;
}

static int finish(lua_State *L)
{
  /*
	void * key;
	lua_pushlightuserdata(L, (void *)&key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	int num = lua_tonumber(L, -1);

	std::string ret;
  */

	int n = lua_gettop(L);
	int i = 0;
	std::string params = "";

	for (i = 1; i <= n; i++)
	{
		/* total the arguments */
                if (lua_type(L, i) != LUA_TSTRING)
		{
		    std::cout << "parameter " << i << ": " << lua_typename(L, lua_type(L, i));
		    continue;
		}
		params += lua_tostring(L, i);
		std::cout << params << std::endl;
	}

	getSmartContractResult(L) = params;
	return n;
}
/**
        scAccountCurrencies function for smart contract
*/
static int scAccountCurrencies(lua_State *L){
        if(lua_gettop(L) < 1){
                return -1;
        }
        std::string str_account = lua_tostring(L, 1u);

        mtchain::RPC::Context& context = getSmartContractContext(L);

        context.params[mtchain::jss::command] = "account_currencies";
        context.params[mtchain::jss::account] = str_account;

        Json::Value j;
        mtchain::RPC::doCommand(context, j);
        std::string result = j.toStyledString();
        lua_pushstring(L, result.c_str());

        return 1;
}
/**
        scAccountObjects function for smart contract
*/
static int scAccountObjects(lua_State *L){
        if(lua_gettop(L) < 1){
                return -1;
        }
        std::string str_account = lua_tostring(L, 1u);

        mtchain::RPC::Context& context = getSmartContractContext(L);

        context.params[mtchain::jss::command] = "account_objects";
        context.params[mtchain::jss::account] = str_account;

        Json::Value j;
        mtchain::RPC::doCommand(context, j);
        std::string result = j.toStyledString();
        lua_pushstring(L, result.c_str());

        return 1;
}
/**
        scAccountOffers function for smart contract
*/
static int scAccountOffers(lua_State *L){
        if(lua_gettop(L) < 1){
                return -1;
        }
        std::string str_account = lua_tostring(L, 1u);

        mtchain::RPC::Context& context = getSmartContractContext(L);

        context.params[mtchain::jss::command] = "account_offers";
        context.params[mtchain::jss::account] = str_account;

        Json::Value j;
        mtchain::RPC::doCommand(context, j);
        std::string result = j.toStyledString();
        lua_pushstring(L, result.c_str());

        return 1;
}
void test_SmartContractResult()
{
	lua_State* L = luaL_newstate();
	std::string result = "OK";
	lua_pushstring(L, "Result");
	lua_pushlightuserdata(L, &result);
	lua_settable(L, LUA_REGISTRYINDEX);
	BOOST_ASSERT(lua_gettop(L) == 0);
	BOOST_ASSERT(&getSmartContractResult(L) == &result);
	std::cout << &getSmartContractResult(L) << " : " << &result
		  << " = " << getSmartContractResult(L) << std::endl;
	lua_close(L);
}

extern "C" int luaopen_lsqlite3(lua_State *L);
lua_State* createLuaVM()
{
    lua_State* L = luaL_newstate();

    if (L == NULL)
    {
        std::cout << "create lua vm failed!" << std::endl;
        return NULL;
    }

    /* load Lua base libraries */
    luaL_openlibs(L);
    luaL_requiref(L, "sqlite3", luaopen_lsqlite3, 1);
    lua_pop(L, 1);

    /* build secure lua environment */
    if (luaL_dofile(L, "./lua/" LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "/sc.init"))
    {
        std::cout << "execute init lua code failed: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, -1);
        lua_close(L);
	return NULL;
    }

    return L;
}

/**
scTx function for smart contract
commandLine format:
	  tx <str_hash>
*/
static int  scTx(lua_State *L)
{
	/*
	get number of arguments
	*/
	if (lua_gettop(L) < 1)
	{
		return -1;
	}
	std::string str_hash = lua_tostring(L, 1u);

	mtchain::RPC::Context& context = getSmartContractContext(L);

	context.params[mtchain::jss::command] = "tx";
	context.params[mtchain::jss::transaction] = str_hash;

	Json::Value j;
	mtchain::RPC::doCommand(context, j);

	std::string result = j.toStyledString();
	JLOG(context.j.debug()) << "result: " << result;

	lua_pushstring(L, result.c_str());

	return 1;
}

int callSmartContract(mtchain::RPC::Context& context, std::string const& sc, std::string &result)
{
	/* initialize Lua */
        lua_State* L = createLuaVM();
	if (L == NULL) {
            JLOG(context.j.warn()) << "create lua vm failed!";
	    return -1;
	}

	// save context info into lua
	lua_pushstring(L, "Context");
	lua_pushlightuserdata(L, &context);
	lua_settable(L, LUA_REGISTRYINDEX);
	BOOST_ASSERT(lua_gettop(L) == 0);
	BOOST_ASSERT(&getSmartContractContext(L) == &context);

	// save result ptr into lua
	lua_pushstring(L, "Result");
	lua_pushlightuserdata(L, &result);
	lua_settable(L, LUA_REGISTRYINDEX);
	BOOST_ASSERT(lua_gettop(L) == 0);
	BOOST_ASSERT(&getSmartContractResult(L) == &result);

	/* register our function */
	lua_register(L, "average", average);
	lua_register(L, "scWalletPropose", scWalletPropose);
	lua_register(L, "scTransfer", scTransfer);
	lua_register(L, "scFrozenAsset", scFrozenAsset);
	lua_register(L, "scUnfrozenAsset", scUnfrozenAsset);
	lua_register(L, "scAccountInfo", scAccountInfo); 
	lua_register(L, "scAccountChannels", scAccountChannels);
	lua_register(L, "scAccountLines", scAccountLines); 
	lua_register(L, "scAccountTx", scAccountTx);
	lua_register(L, "finish", finish);
	lua_register(L, "init", init);
	lua_register(L, "init_s", init_s);
	lua_register(L, "scLedgerClosed", scLedgerClosed);        
	lua_register(L, "scSimLedgerAccept", scSimLedgerAccept);        
	lua_register(L, "scSimLedgerCurrent", scSimLedgerCurrent);        
	lua_register(L, "scSimLedgerRequest", scSimLedgerRequest);        
	lua_register(L, "scAccountCurrencies", scAccountCurrencies);
	lua_register(L, "scAccountObjects", scAccountObjects);
	lua_register(L, "scAccountOffers", scAccountOffers);
	lua_register(L, "scServerInfo",scServerInfo);
	lua_register(L, "scTx",scTx);
	lua_register(L, "scLedger",scLedger);

	/* run the script */
	//luaL_dofile(L, "avg.lua");
	int ret = luaL_dostring(L, sc.c_str()); // It returns false if there are no errors or true in case of errors.
	if (ret)
	{
	    JLOG(context.j.warn()) << "exec smart contract '" << sc << "' failed: "
				   << lua_tostring(L, -1);
	}

	//int n = lua_gettop(L);
	//std::string result = lua_tostring(L, -1);

	/* cleanup Lua */
	lua_close(L);

	return ret;
}

namespace mtchain {
extern Application& getApp();
}
int testSmartContract(char const* code)
{
    auto &app = mtchain::getApp();
    auto j = app.journal("SmartContract");
    Json::Value jvCommand;
    jvCommand[mtchain::jss::command] = "sc";
    mtchain::Resource::Charge loadType = mtchain::Resource::feeReferenceRPC;
    mtchain::Resource::Consumer c;
    mtchain::RPC::Context context { j, jvCommand, app, loadType, app.getOPs(),
	app.getLedgerMaster(), c, mtchain::Role::ADMIN };

    std::string result;
    int ret = callSmartContract(context, code, result);
    if (!ret && !result.empty())
    {
        JLOG(context.j.warn()) << "smart contract '" << code << "' result: " << result;
    }

    return ret;
}

std::string loadSmartContract(mtchain::RPC::Context& context, std::string const& tx_hash)
{
#if 0
	std::vector<std::string> const& args = { "tx", "2BCDFD0A8EB68C1EB3F8F1422B8D4F22B3D06EFA47F2B62103504576995918D0" };

	auto config = std::make_unique<mtchain::Config>();
	config->setup("", false, false, false);

	beast::severities::Severity thresh = beast::severities::Severity::kInfo;
	auto logs = std::make_unique<mtchain::Logs>(thresh);
#endif
	// use rpcClient
	///std::pair<int, Json::Value> jvOut = rpcClient(args, *config, *logs);
	//Json::Value j = jvOut.second;

	Json::Value j;
	//Json::Value params();
	context.params[mtchain::jss::command] = "tx";
	//context.params[mtchain::jss::transaction] = "2BCDFD0A8EB68C1EB3F8F1422B8D4F22B3D06EFA47F2B62103504576995918D0";
	mtchain::RPC::doCommand(context, j);

	JLOG(context.j.debug()) << "result: " << j.toStyledString();

// 	std::cout << "---------Memos--------------";
// 	std::cout << j["Memos"].toStyledString();
	JLOG(context.j.debug()) << "Memos: " << j["Memos"].toStyledString();


	std::string memo_type = j["Memos"][0u]["Memo"]["MemoType"].asString(); // get array must use index with 'u'
	//std::cout << memo_type << std::endl;
	JLOG(context.j.debug()) << "memo_type: " << memo_type;

	if (memo_type.size() % 2)
	{
		memo_type.append(" ");
	}
	std::pair<mtchain::Blob, bool> data = mtchain::strUnHex(memo_type);
	if (data.second)
	{
		mtchain::Serializer s(data.first.data(), data.first.size());
		std::cout << s.getString() << std::endl;
	}

	std::string memo_data = j["Memos"][0u]["Memo"]["MemoData"].asString(); // get array must use index with 'u'
	std::cout << memo_data << std::endl;

	if (memo_data.size() % 2)
	{
		memo_data.append(" ");
	}
	std::string sc = "";
	data = mtchain::strUnHex(memo_data);
	if (data.second)
	{
		mtchain::Serializer s(data.first.data(), data.first.size());
		std::cout << s.getString() << std::endl;
	}
	sc = mtchain::scDecode(memo_data);
	std::cout << sc << std::endl;

	return sc;
}

int call_smart_contract(mtchain::RPC::Context& context, std::string &result)
{
	std::string sc = loadSmartContract(context,"tx hash");

	return callSmartContract(context, sc, result);
}

namespace mtchain {

//extern bool test_smart_contract();


Json::Value doSmartContract (RPC::Context& context)
{
    return smartContract (context);
}

Json::Value smartContract (mtchain::RPC::Context& context)
{
    Json::Value ret;
    std::string result;

    size_t cnt = scCounters.count(0);
    if (call_smart_contract(context, result))
    {
        ret[jss::smart_contract] = "call smart contract fail";
    }
    else
    {
        ret[jss::smart_contract] = result;
    }

    scCounters.count(1);
    if (cnt > 1)
    {
        JLOG(context.j.warn()) << "There are " << cnt <<
		" smart contracts which are running and it has occured " <<
		scCounters.getCounterHistory(cnt) << " times!";
    }
    
    return ret;
}

std::string scEncode(std::string const& str)
{
	std::string hex = "";
	hex = strHex(str);

	return hex;
}

std::string scDecode(std::string& hex)
{
	std::string str = "";

	if (hex.size() % 2)
	{
		hex.append(" ");
	}
	std::pair<Blob, bool> data = strUnHex(hex);
	if (data.second)
	{
		Serializer s(data.first.data(), data.first.size());
		std::cout << s.getString() << std::endl;
		str = s.getString();
	}

	return str;
}

} //
