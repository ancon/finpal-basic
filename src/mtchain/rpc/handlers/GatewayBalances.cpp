//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/paths/MTChainState.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/AccountID.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/basics/Log.h>

namespace mtchain {



// Query:
// 1) Specify ledger to query.
// 2) Specify issuer account (cold wallet) in "account" field.
// 3) Specify accounts that hold gateway assets (such as hot wallets)
//    using "hotwallet" field which should be either a string (if just
//    one wallet) or an array of strings (if more than one).

// Response:
// 1) Array, "obligations", indicating the total obligations of the
//    gateway in each currency. Obligations to specified hot wallets
//    are not counted here.
// 2) Object, "balances", indicating balances in each account
//    that holds gateway assets. (Those specified in the "hotwallet"
//    field.)
// 3) Object of "assets" indicating accounts that owe the gateway.
//    (Gateways typically do not hold positive balances. This is unusual.)

// gateway_balances [<ledger>] <account> [<howallet> [<hotwallet [...

Json::Value doGatewayBalances (RPC::Context& context)
{
    auto& params = context.params;

    // Get the current ledger
    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    if (!(params.isMember (jss::account) || params.isMember (jss::ident)))
        return RPC::missing_field_error (jss::account);

    std::string const strIdent (params.isMember (jss::account)
        ? params[jss::account].asString ()
        : params[jss::ident].asString ());

    bool const bStrict = params.isMember (jss::strict) &&
            params[jss::strict].asBool ();

    // Get info on account.
    AccountID accountID;
    auto jvAccepted = RPC::accountFromString (accountID, strIdent, bStrict);

    if (jvAccepted)
        return jvAccepted;

    context.loadType = Resource::feeHighBurdenRPC;

    result[jss::account] = context.app.accountIDCache().toBase58 (accountID);

    // Parse the specified hotwallet(s), if any
    std::set <AccountID> hotWallets;

    if (params.isMember (jss::hotwallet))
    {

        auto addHotWallet = [&hotWallets](Json::Value const& j)
        {
            if (j.isString())
            {
                auto const pk = parseBase58<PublicKey>(
                    TokenType::TOKEN_ACCOUNT_PUBLIC,
                    j.asString ());
                if (pk)
                {
                    hotWallets.insert(calcAccountID(*pk));
                    return true;
                }

                auto const id = parseBase58<AccountID>(j.asString());

                if (id)
                {
                    hotWallets.insert(*id);
                    return true;
                }
            }

            return false;
        };

        Json::Value const& hw = params[jss::hotwallet];
        bool valid = true;

        if (hw.isArray())
        {
            for (unsigned i = 0; i < hw.size(); ++i)
                valid &= addHotWallet (hw[i]);
        }
        else if (hw.isString())
        {
            valid &= addHotWallet (hw);
        }
        else
        {
            valid = false;
        }

        if (! valid)
        {
            result[jss::error]   = "invalidHotWallet";
            return result;
        }

    }

    std::map <Currency, STAmount> sums;
    std::map <AccountID, std::vector <STAmount>> hotBalances;
    std::map <AccountID, std::vector <STAmount>> assets;
    std::map <AccountID, std::vector <STAmount>> frozenBalances;

    // Traverse the cold wallet's trust lines
    {
        forEachItem(*ledger, accountID,
            [&](std::shared_ptr<SLE const> const& sle)
            {
                auto rs = MTChainState::makeItem (accountID, sle);

                if (!rs)
                    return;

                int balSign = rs->getBalance().signum();
                if (balSign == 0)
                    return;

                auto const& peer = rs->getAccountIDPeer();

                // Here, a negative balance means the cold wallet owes (normal)
                // A positive balance means the cold wallet has an asset (unusual)

                if (hotWallets.count (peer) > 0)
                {
                    // This is a specified hot wallet
                    hotBalances[peer].push_back (-rs->getBalance ());
                }
                else if (balSign > 0)
                {
                    // This is a gateway asset
                    assets[peer].push_back (rs->getBalance ());
                }
                else if (rs->getFreeze())
                {
                    // An obligation the gateway has frozen
                    frozenBalances[peer].push_back (-rs->getBalance ());
                }
                else
                {
                    // normal negative balance, obligation to customer
                    auto& bal = sums[rs->getBalance().getCurrency()];
                    if (bal == zero)
                    {
                        // This is needed to set the currency code correctly
                        bal = -rs->getBalance();
                    }
                    else
                        bal -= rs->getBalance();
                }
            });
    }

    if (! sums.empty())
    {
        Json::Value j;
        for (auto const& e : sums)
        {
            j[to_string (e.first)] = e.second.getText ();
			JLOG(context.j.debug()) << 
				" Currency : " << e.first << 
				" Value : " << e.second.getText() << 
				" Signum : " << e.second.signum() <<
				" Mantissa : " << e.second.mantissa() <<
				" Exponent : " << e.second.exponent() <<
				" Native : " << e.second.native() <<
				" Negative : " << e.second.negative() <<
				" STAmount : " << e.second.getFullText();
        }
        result [jss::obligations] = std::move (j);
    }

    auto populate = [](
        std::map <AccountID, std::vector <STAmount>> const& array,
        Json::Value& result,
        Json::StaticString const& name)
        {
            if (!array.empty())
            {
                Json::Value j;
                for (auto const& account : array)
                {
                    Json::Value balanceArray;
                    for (auto const& balance : account.second)
                    {
                        Json::Value entry;
                        entry[jss::currency] = to_string (balance.issue ().currency);
                        entry[jss::value] = balance.getText();
                        balanceArray.append (std::move (entry));
                    }
                    j [to_string (account.first)] = std::move (balanceArray);
                }
                result [name] = std::move (j);
            }
        };

    populate (hotBalances, result, jss::balances);
    populate (frozenBalances, result, jss::frozen_balances);
    populate (assets, result, jss::assets);

	//////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_GATEWAY_BALANCE


	JLOG(context.j.debug()) << result.toStyledString() << std::endl;

	std::string token = "CNY";
	Json::Value obligations = result[jss::obligations];


	JLOG(context.j.debug()) << obligations[token] << std::endl;

	Json::Value  balance = obligations.get(token, NULL);
	JLOG(context.j.debug()) << "balance = " <<balance << std::endl;


	std::string const& issuer ="6nUy2SHT6B9DubsPmkJZUXTf5FcNDG6YEA";
	std::string const& currency = "CAD";
	STAmount iou = getGatewayAmount(issuer, currency);
	JLOG(context.j.debug()) <<
		"IOU Currency : " << iou.getCurrency() <<
		" Value : " << iou.getText() <<
		" Signum : " << iou.signum() <<
		" Mantissa : " << iou.mantissa() <<
		" Exponent : " << iou.exponent() <<
		" Native : " << iou.native() <<
		" Negative : " << iou.negative() <<
		" STAmount : " << iou.getFullText();

#endif // DEBUG_GATEWAY_BALANCE
	//////////////////////////////////////////////////////////////////////////
    return result;
}

#ifdef DEBUG_GATEWAY_BALANCE
extern Application& getApp();

STAmount
getGatewayAmount(std::string const& issuer, std::string const& currency)
{
	auto &app = getApp();
	auto j = app.journal("RPCHandler");
	Json::Value jvCommand;
	jvCommand[mtchain::jss::command] = "gateway_balances";
	mtchain::Resource::Charge loadType = mtchain::Resource::feeReferenceRPC;
	mtchain::Resource::Consumer c;
	mtchain::RPC::Context context{ j, jvCommand, app, loadType, app.getOPs(),
		app.getLedgerMaster(), c, mtchain::Role::ADMIN };

	auto& params = context.params;

	params[jss::account] = issuer;
#ifdef DEBUG_GATEWAY_BALANCE
	params[jss::account] = "6nUy2SHT6B9DubsPmkJZUXTf5FcNDG6YEA";
#endif // DEBUG_GATEWAY_BALANCE

	// Get the current ledger
	std::shared_ptr<ReadView const> ledger;
	auto result = RPC::lookupLedger(ledger, context);

	if (!ledger)
		return NULL;

	if (!(params.isMember(jss::account) || params.isMember(jss::ident)))
		return NULL;

	std::string const strIdent(params.isMember(jss::account)
		? params[jss::account].asString()
		: params[jss::ident].asString());

	bool const bStrict = params.isMember(jss::strict) &&
		params[jss::strict].asBool();

	// Get info on account.
	AccountID accountID;
	auto jvAccepted = RPC::accountFromString(accountID, strIdent, bStrict);

	if (jvAccepted)
		return NULL;
	// Parse the specified hotwallet(s), if any
	std::set <AccountID> hotWallets;

	if (params.isMember(jss::hotwallet))
	{

		auto addHotWallet = [&hotWallets](Json::Value const& j)
		{
			if (j.isString())
			{
				auto const pk = parseBase58<PublicKey>(
					TokenType::TOKEN_ACCOUNT_PUBLIC,
					j.asString());
				if (pk)
				{
					hotWallets.insert(calcAccountID(*pk));
					return true;
				}

				auto const id = parseBase58<AccountID>(j.asString());

				if (id)
				{
					hotWallets.insert(*id);
					return true;
				}
			}

			return false;
		};

		Json::Value const& hw = params[jss::hotwallet];
		bool valid = true;

		if (hw.isArray())
		{
			for (unsigned i = 0; i < hw.size(); ++i)
				valid &= addHotWallet(hw[i]);
		}
		else if (hw.isString())
		{
			valid &= addHotWallet(hw);
		}
		else
		{
			valid = false;
		}

		if (!valid)
		{
			result[jss::error] = "invalidHotWallet";
			return NULL;
		}

	}

	std::map <Currency, STAmount> sums;
	std::map <AccountID, std::vector <STAmount>> hotBalances;
	std::map <AccountID, std::vector <STAmount>> assets;
	std::map <AccountID, std::vector <STAmount>> frozenBalances;

	// Traverse the cold wallet's trust lines
	{
		forEachItem(*ledger, accountID,
			[&](std::shared_ptr<SLE const> const& sle)
		{
			auto rs = MTChainState::makeItem(accountID, sle);

			if (!rs)
				return;

			int balSign = rs->getBalance().signum();
			if (balSign == 0)
				return;

			auto const& peer = rs->getAccountIDPeer();

			// Here, a negative balance means the cold wallet owes (normal)
			// A positive balance means the cold wallet has an asset (unusual)

			if (hotWallets.count(peer) > 0)
			{
				// This is a specified hot wallet
				hotBalances[peer].push_back(-rs->getBalance());
			}
			else if (balSign > 0)
			{
				// This is a gateway asset
				assets[peer].push_back(rs->getBalance());
			}
			else if (rs->getFreeze())
			{
				// An obligation the gateway has frozen
				frozenBalances[peer].push_back(-rs->getBalance());
			}
			else
			{
				// normal negative balance, obligation to customer
				auto& bal = sums[rs->getBalance().getCurrency()];
				if (bal == zero)
				{
					// This is needed to set the currency code correctly
					bal = -rs->getBalance();
				}
				else
					bal -= rs->getBalance();
			}
		});
	}

	if (!sums.empty())
	{
		Json::Value j;
		for (auto const& e : sums)
		{
			j[to_string(e.first)] = e.second.getText();
			JLOG(context.j.debug()) <<
				" Currency : " << e.first <<
				" Value : " << e.second.getText() <<
				" Signum : " << e.second.signum() <<
				" Mantissa : " << e.second.mantissa() <<
				" Exponent : " << e.second.exponent() <<
				" Native : " << e.second.native() <<
				" Negative : " << e.second.negative() <<
				" STAmount : " << e.second.getFullText();
			
			if (currency == to_string(e.first))
			{
				return e.second;
			}
		}
		//result[jss::obligations] = std::move(j);
	}

	return NULL;
}
#endif

#ifdef DEBUG_GATEWAY_BALANCE
std::uint64_t getGatewayBalance(std::string token)
{

	auto &app = getApp();
	auto j = app.journal("RPCHandler");
	Json::Value jvCommand;
	jvCommand[mtchain::jss::command] = "gateway_balances";
	mtchain::Resource::Charge loadType = mtchain::Resource::feeReferenceRPC;
	mtchain::Resource::Consumer c;
	//mtchain::RPC::Context context{ j, jvCommand, app, loadType, app.getOPs(),
	//	app.getLedgerMaster(), c, mtchain::Role::ADMIN };
	mtchain::RPC::Context context{ j, jvCommand, app, loadType, app.getOPs(),
		app.getLedgerMaster(), c, mtchain::Role::ADMIN };

	Json::Value gbs = doGatewayBalances(context);
	Json::Value obligations = gbs[jss::result][jss::obligations];
	Json::Value  balance = obligations.get(token, NULL);
	return balance.asDouble();

}

#endif // DEBUG_GATEWAY_BALANCE


} //
