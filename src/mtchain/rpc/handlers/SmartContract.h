//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.
*/
//==============================================================================

#ifndef MTCHAIN_RPC_HANDLERS_SMARTCONTRACT_H
#define MTCHAIN_RPC_HANDLERS_SMARTCONTRACT_H

#include <mtchain/json/json_value.h>

#include <BeastConfig.h>
#include <mtchain/basics/Log.h>
#include <test/jtx.h>
#include <mtchain/json/to_string.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/TxFlags.h>
#include <mtchain/beast/hash/uhash.h>
#include <mtchain/beast/unit_test.h>
#include <boost/lexical_cast.hpp>
#include <utility>

extern "C" {
#include "../../../slua/include/lua.h"
#include "../../../slua/include/lualib.h"
#include "../../../slua/include/lauxlib.h"
}


namespace mtchain {

Json::Value smartContract (mtchain::RPC::Context& context);

std::string scEncode(std::string const& str);
std::string scDecode(std::string& hex);

namespace test {

class Env_SmartContract_test : public beast::unit_test::suite
{
	public:
		template <class T>
		static
			std::string
			to_string(T const& t)
		{
			return boost::lexical_cast<
				std::string>(t);
		}

		// Test Env
		void testEnv()
		{
			using namespace jtx;
			auto const n = M(10000);
			auto const gw = Account("gw");
			auto const USD = gw["USD"];
			auto const alice = Account("alice");

			// unfunded
			{
				// During Env construct, a pointer "runner" is null ,so can not send tx success
				Env env(*this);

				auto const info = env.rpc("json", "account_info", "{\"account\": "
					"\"6Hb9CJAWyB46j91VRWn9rDkukG4bwdtyTh\"}");

				//env(pay("alice", "bob", M(1000)), seq(1), fee(10), sig("alice"), ter(terNO_ACCOUNT));
				//env.require(balance("alice", M(10000)));
				//env(pay(env.master, "bob", M(10)));
				env(pay(env.master, "bob", M(1000)), fee(100),env.seq(env.master), sig(env.master));
			}

		}

		// Payment basics
		void
			testPayments()
		{
			using namespace jtx;
			Env env(*this);
			auto const gw = Account("gateway");
			auto const USD = gw["USD"];

			env.fund(M(10000), "alice", "bob", "carol", gw);
			env.require(balance("alice", M(10000)));
			env.require(balance("bob", M(10000)));
			env.require(balance("carol", M(10000)));
			env.require(balance(gw, M(10000)));

			env(pay("alice", "bob", M(8)), fee(100), sig("alice"));

			env(pay(env.master, "alice", M(1000)), fee(none), ter(temMALFORMED));
			env(pay(env.master, "alice", M(1000)), fee(1), ter(telINSUF_FEE_P));
			env(pay(env.master, "alice", M(1000)), seq(none), ter(temMALFORMED));
			env(pay(env.master, "alice", M(1000)), seq(20), ter(terPRE_SEQ));
			env(pay(env.master, "alice", M(1000)), sig(none), ter(temMALFORMED));
			env(pay(env.master, "alice", M(1000)), sig("bob"), ter(tefBAD_AUTH_MASTER));

			env(pay(env.master, "dilbert", M(1000)), sig(env.master));
		}

		void
			run()
		{
			testEnv();
			testPayments();
		}
	};

BEAST_DEFINE_TESTSUITE(Env_SmartContract, app, mtchain);

} // test
} //

#endif
