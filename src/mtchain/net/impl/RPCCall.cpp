//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/net/RPCCall.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/json/json_reader.h>
#include <mtchain/json/to_string.h>
#include <mtchain/json/Object.h>
#include <mtchain/net/HTTPClient.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/SystemParameters.h>
#include <mtchain/protocol/types.h>
#include <mtchain/rpc/ServerHandler.h>
#include <mtchain/beast/core/LexicalCast.h>
#include <beast/core/detail/ci_char_traits.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/regex.hpp>
#include <array>
#include <iostream>
#include <type_traits>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

class RPCParser;

//
// HTTP protocol
//
// This ain't Apache.  We're just using HTTP header for the length field
// and to be compatible with other JSON-RPC implementations.
//

std::string createHTTPPost (
    std::string const& strHost,
    std::string const& strPath,
    std::string const& strMsg,
    std::map<std::string, std::string> const& mapRequestHeaders)
{
    std::ostringstream s;

    // CHECKME this uses a different version than the replies below use. Is
    //         this by design or an accident or should it be using
    //         BuildInfo::getFullVersionString () as well?

    s << "POST "
      << (strPath.empty () ? "/" : strPath)
      << " HTTP/1.0\r\n"
      << "User-Agent: " << systemName () << "-json-rpc/v1\r\n"
      << "Host: " << strHost << "\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << strMsg.size () << "\r\n"
      << "Accept: application/json\r\n";

    for (auto const& item : mapRequestHeaders)
        s << item.first << ": " << item.second << "\r\n";

    s << "\r\n" << strMsg;

    return s.str ();
}

class RPCParser
{
private:
    beast::Journal j_;
    Config const& config_;

    // TODO New routine for parsing ledger parameters, other routines should standardize on this.
    static bool jvParseLedger (Json::Value& jvRequest, std::string const& strLedger)
    {
        if (strLedger == "current" || strLedger == "closed" || strLedger == "validated")
        {
            jvRequest[jss::ledger_index]   = strLedger;
        }
        else if (strLedger.length () == 64)
        {
            // YYY Could confirm this is a uint256.
            jvRequest[jss::ledger_hash]    = strLedger;
        }
        else
        {
            // beast::lexicalCast can be used to parse numeric string to number
            jvRequest[jss::ledger_index]   = beast::lexicalCast <std::uint32_t> (strLedger);
        }

        return true;
    }

    // Build a object { "currency" : "XYZ", "issuer" : "rXYX" }
    static Json::Value jvParseCurrencyIssuer (std::string const& strCurrencyIssuer)
    {
        static boost::regex reCurIss ("\\`([[:alpha:]]{3})(?:/(.+))?\\'");

        boost::smatch   smMatch;

        if (boost::regex_match (strCurrencyIssuer, smMatch, reCurIss))
        {
            Json::Value jvResult (Json::objectValue);
            std::string strCurrency = smMatch[1];
            std::string strIssuer   = smMatch[2];

            jvResult[jss::currency]    = strCurrency;

            if (strIssuer.length ())
            {
                // Could confirm issuer is a valid MTChain address.
                jvResult[jss::issuer]      = strIssuer;
            }

            return jvResult;
        }
        else
        {
            return RPC::make_param_error (std::string ("Invalid currency/issuer '") +
                    strCurrencyIssuer + "'");
        }
    }

private:
    using parseFuncPtr = Json::Value (RPCParser::*) (Json::Value const& jvParams);

    Json::Value parseAsIs (Json::Value const& jvParams)
    {
        Json::Value v (Json::objectValue);

        if (jvParams.isArray () && (jvParams.size () > 0))
            v[jss::params] = jvParams;

        return v;
    }

    Json::Value parseInternal (Json::Value const& jvParams)
    {
        Json::Value v (Json::objectValue);
        v[jss::internal_command] = jvParams[0u];

        Json::Value params (Json::arrayValue);

        for (unsigned i = 1; i < jvParams.size (); ++i)
            params.append (jvParams[i]);

        v[jss::params] = params;

        return v;
    }

    // fetch_info [clear]
    Json::Value parseFetchInfo (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);
        unsigned int    iParams = jvParams.size ();

        if (iParams != 0)
            jvRequest[jvParams[0u].asString()] = true;

        return jvRequest;
    }

    // account_tx accountID [ledger_min [ledger_max [limit [offset]]]] [binary] [count] [descending]
    Json::Value
    parseAccountTransactions (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);
        unsigned int    iParams = jvParams.size ();

        auto const account =
            parseBase58<AccountID>(jvParams[0u].asString());
        if (! account)
            return rpcError (rpcACT_MALFORMED);

        jvRequest[jss::account]= toBase58(*account);

        bool            bDone   = false;

        while (!bDone && iParams >= 2)
        {
            // VFALCO Why is Json::StaticString appearing on the right side?
            if (jvParams[iParams - 1].asString () == jss::binary)
            {
                jvRequest[jss::binary]     = true;
                --iParams;
            }
            else if (jvParams[iParams - 1].asString () == jss::count)
            {
                jvRequest[jss::count]      = true;
                --iParams;
            }
            else if (jvParams[iParams - 1].asString () == jss::descending)
            {
                jvRequest[jss::descending] = true;
                --iParams;
            }
            else
            {
                bDone   = true;
            }
        }

        if (1 == iParams)
        {
        }
        else if (2 == iParams)
        {
            if (!jvParseLedger (jvRequest, jvParams[1u].asString ()))
                return jvRequest;
        }
        else
        {
            std::int64_t   uLedgerMin  = jvParams[1u].asInt ();
            std::int64_t   uLedgerMax  = jvParams[2u].asInt ();

            if (uLedgerMax != -1 && uLedgerMax < uLedgerMin)
            {
                return rpcError (rpcLGR_IDXS_INVALID);
            }

            jvRequest[jss::ledger_index_min]   = jvParams[1u].asInt ();
            jvRequest[jss::ledger_index_max]   = jvParams[2u].asInt ();

            if (iParams >= 4)
                jvRequest[jss::limit]  = jvParams[3u].asInt ();

            if (iParams >= 5)
                jvRequest[jss::offset] = jvParams[4u].asInt ();
        }

        return jvRequest;
    }

    // tx_account accountID [ledger_min [ledger_max [limit]]]] [binary] [count] [forward]
    Json::Value parseTxAccount (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);
        unsigned int    iParams = jvParams.size ();

        auto const account =
            parseBase58<AccountID>(jvParams[0u].asString());
        if (! account)
            return rpcError (rpcACT_MALFORMED);

        jvRequest[jss::account]    = toBase58(*account);

        bool            bDone   = false;

        while (!bDone && iParams >= 2)
        {
            if (jvParams[iParams - 1].asString () == jss::binary)
            {
                jvRequest[jss::binary]     = true;
                --iParams;
            }
            else if (jvParams[iParams - 1].asString () == jss::count)
            {
                jvRequest[jss::count]      = true;
                --iParams;
            }
            else if (jvParams[iParams - 1].asString () == jss::forward)
            {
                jvRequest[jss::forward] = true;
                --iParams;
            }
            else
            {
                bDone   = true;
            }
        }

        if (1 == iParams)
        {
        }
        else if (2 == iParams)
        {
            if (!jvParseLedger (jvRequest, jvParams[1u].asString ()))
                return jvRequest;
        }
        else
        {
            std::int64_t   uLedgerMin  = jvParams[1u].asInt ();
            std::int64_t   uLedgerMax  = jvParams[2u].asInt ();

            if (uLedgerMax != -1 && uLedgerMax < uLedgerMin)
            {
                return rpcError (rpcLGR_IDXS_INVALID);
            }

            jvRequest[jss::ledger_index_min]   = jvParams[1u].asInt ();
            jvRequest[jss::ledger_index_max]   = jvParams[2u].asInt ();

            if (iParams >= 4)
                jvRequest[jss::limit]  = jvParams[3u].asInt ();
        }

        return jvRequest;
    }

    // book_offers <taker_pays> <taker_gets> [<taker> [<ledger> [<limit> [<proof> [<marker>]]]]]
    // limit: 0 = no limit
    // proof: 0 or 1
    //
    // Mnemonic: taker pays --> offer --> taker gets
    Json::Value parseBookOffers (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        Json::Value     jvTakerPays = jvParseCurrencyIssuer (jvParams[0u].asString ());
        Json::Value     jvTakerGets = jvParseCurrencyIssuer (jvParams[1u].asString ());

        if (isRpcError (jvTakerPays))
        {
            return jvTakerPays;
        }
        else
        {
            jvRequest[jss::taker_pays] = jvTakerPays;
        }

        if (isRpcError (jvTakerGets))
        {
            return jvTakerGets;
        }
        else
        {
            jvRequest[jss::taker_gets] = jvTakerGets;
        }

        if (jvParams.size () >= 3)
        {
            jvRequest[jss::issuer] = jvParams[2u].asString ();
        }

        if (jvParams.size () >= 4 && !jvParseLedger (jvRequest, jvParams[3u].asString ()))
            return jvRequest;

        if (jvParams.size () >= 5)
        {
            int     iLimit  = jvParams[5u].asInt ();

            if (iLimit > 0)
                jvRequest[jss::limit]  = iLimit;
        }

        if (jvParams.size () >= 6 && jvParams[5u].asInt ())
        {
            jvRequest[jss::proof]  = true;
        }

        if (jvParams.size () == 7)
            jvRequest[jss::marker] = jvParams[6u];

        return jvRequest;
    }

    // can_delete [<ledgerid>|<ledgerhash>|now|always|never]
    Json::Value parseCanDelete (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        if (!jvParams.size ())
            return jvRequest;

        std::string input = jvParams[0u].asString();
        if (input.find_first_not_of("0123456789") ==
                std::string::npos)
            jvRequest["can_delete"] = jvParams[0u].asUInt();
        else
            jvRequest["can_delete"] = input;

        return jvRequest;
    }

    // connect <ip> [port]
    Json::Value parseConnect (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        jvRequest[jss::ip] = jvParams[0u].asString ();

        if (jvParams.size () == 2)
            jvRequest[jss::port]   = jvParams[1u].asUInt ();

        return jvRequest;
    }

    // Return an error for attemping to subscribe/unsubscribe via RPC.
    Json::Value parseEvented (Json::Value const& jvParams)
    {
        return rpcError (rpcNO_EVENTS);
    }

    // feature [<feature>] [accept|reject]
    Json::Value parseFeature (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        if (jvParams.size () > 0)
            jvRequest[jss::feature] = jvParams[0u].asString ();

        if (jvParams.size () > 1)
        {
            auto const action = jvParams[1u].asString ();

            // This may look reversed, but it's intentional: jss::vetoed
            // determines whether an amendment is vetoed - so "reject" means
            // that jss::vetoed is true.
            if (beast::detail::ci_equal(action, "reject"))
                jvRequest[jss::vetoed] = Json::Value (true);
            else if (beast::detail::ci_equal(action, "accept"))
                jvRequest[jss::vetoed] = Json::Value (false);
            else
                return rpcError (rpcINVALID_PARAMS);
        }

        return jvRequest;
    }

    // get_counts [<min_count>]
    Json::Value parseGetCounts (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        if (jvParams.size ())
            jvRequest[jss::min_count]  = jvParams[0u].asUInt ();

        return jvRequest;
    }

    // sign_for <account> <secret> <json> offline
    // sign_for <account> <secret> <json>
    Json::Value parseSignFor (Json::Value const& jvParams)
    {
        bool const bOffline = 4 == jvParams.size () && jvParams[3u].asString () == "offline";

        if (3 == jvParams.size () || bOffline)
        {
            Json::Value txJSON;
            Json::Reader reader;
            if (reader.parse (jvParams[2u].asString (), txJSON))
            {
                // sign_for txJSON.
                Json::Value jvRequest;

                jvRequest[jss::account] = jvParams[0u].asString ();
                jvRequest[jss::secret]  = jvParams[1u].asString ();
                jvRequest[jss::tx_json] = txJSON;

                if (bOffline)
                    jvRequest[jss::offline] = true;

                return jvRequest;
            }
        }
        return rpcError (rpcINVALID_PARAMS);
    }
		//do not request any parameters
//ipfs_fee_info
    Json::Value parseIpfsFeeInfo(Json::Value const& jvParams){
    Json::Value jvRequest(Json::objectValue);	
    if(jvParams.size() > 0)
    {
        return rpcError(rpcINVALID_PARAMS);
    }
    return jvRequest;
    }

    // json <command> <json>
    Json::Value parseJson (Json::Value const& jvParams)
    {
        Json::Reader    reader;
        Json::Value     jvRequest;

        JLOG (j_.trace()) << "RPC method: " << jvParams[0u];
        JLOG (j_.trace()) << "RPC json: " << jvParams[1u];

        if (reader.parse (jvParams[1u].asString (), jvRequest))
        {
            if (!jvRequest.isObject ())
                return rpcError (rpcINVALID_PARAMS);

            jvRequest[jss::method] = jvParams[0u];

            return jvRequest;
        }

        return rpcError (rpcINVALID_PARAMS);
    }

    bool isValidJson2(Json::Value const& jv)
    {
        if (jv.isObject())
        {
            if (jv.isMember(jss::jsonrpc) && jv[jss::jsonrpc] == "2.0" &&
                jv.isMember(jss::FinPalrpc) && jv[jss::FinPalrpc] == "2.0" &&
                jv.isMember(jss::id) && jv.isMember(jss::method))
            {
                if (jv.isMember(jss::params) &&
                      !(jv[jss::params].isArray() || jv[jss::params].isNull()))
                    return false;
                return true;
            }
        }
        return false;
    }

    Json::Value parseJson2(Json::Value const& jvParams)
    {
        Json::Reader reader;
        Json::Value jv;
        bool valid_parse = reader.parse(jvParams[0u].asString(), jv);
        if (valid_parse && isValidJson2(jv))
        {
            Json::Value jv1{Json::objectValue};
            if (jv.isMember(jss::params))
            {
                auto const& params = jv[jss::params][0u];
                for (auto i = params.begin(); i != params.end(); ++i)
                    jv1[i.key().asString()] = *i;
            }
            jv1[jss::jsonrpc] = jv[jss::jsonrpc];
            jv1[jss::FinPalrpc] = jv[jss::FinPalrpc];
            jv1[jss::id] = jv[jss::id];
            jv1[jss::method] = jv[jss::method];
            return jv1;
        }
        auto jv_error = rpcError(rpcINVALID_PARAMS);
        if (jv.isMember(jss::jsonrpc))
            jv_error[jss::jsonrpc] = jv[jss::jsonrpc];
        if (jv.isMember(jss::FinPalrpc))
            jv_error[jss::FinPalrpc] = jv[jss::FinPalrpc];
        if (jv.isMember(jss::id))
            jv_error[jss::id] = jv[jss::id];
        return jv_error;
    }

    // ledger [id|index|current|closed|validated] [full|tx]
    Json::Value parseLedger (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        if (!jvParams.size ())
        {
            return jvRequest;
        }

        jvParseLedger (jvRequest, jvParams[0u].asString ());

        if (2 == jvParams.size ())
        {
            if (jvParams[1u].asString () == "full")
            {
                jvRequest[jss::full]   = true;
            }
            else if (jvParams[1u].asString () == "tx")
            {
                jvRequest[jss::transactions] = true;
                jvRequest[jss::expand] = true;
            }
        }

        return jvRequest;
    }

    // ledger_header <id>|<index>
    Json::Value parseLedgerId (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        std::string     strLedger   = jvParams[0u].asString ();

        if (strLedger.length () == 64)
        {
            jvRequest[jss::ledger_hash]    = strLedger;
        }
        else
        {
            jvRequest[jss::ledger_index]   = beast::lexicalCast <std::uint32_t> (strLedger);
        }

        return jvRequest;
    }

    // log_level:                           Get log levels
    // log_level <severity>:                Set master log level to the specified severity
    // log_level <partition> <severity>:    Set specified partition to specified severity
    Json::Value parseLogLevel (Json::Value const& jvParams)
    {
        Json::Value     jvRequest (Json::objectValue);

        if (jvParams.size () == 1)
        {
            jvRequest[jss::severity] = jvParams[0u].asString ();
        }
        else if (jvParams.size () == 2)
        {
            jvRequest[jss::partition] = jvParams[0u].asString ();
            jvRequest[jss::severity] = jvParams[1u].asString ();
        }

        return jvRequest;
    }

    // owner_info <account>|<account_public_key>
    // owner_info <seed>|<pass_phrase>|<key> [<ledfer>]
    // account_info <account>|<account_public_key>
    // account_info <seed>|<pass_phrase>|<key> [<ledger>]
    // account_offers <account>|<account_public_key> [<ledger>]
    Json::Value parseAccountItems (Json::Value const& jvParams)
    {
        return parseAccountRaw1 (jvParams);
    }

    Json::Value parseAccountCurrencies (Json::Value const& jvParams)
    {
        return parseAccountRaw1 (jvParams);
    }

    // account_lines <account> <account>|"" [<ledger>]
    Json::Value parseAccountLines (Json::Value const& jvParams)
    {
        return parseAccountRaw2 (jvParams, jss::peer);
    }

    // account_channels <account> <account>|"" [<ledger>]
    Json::Value parseAccountChannels (Json::Value const& jvParams)
    {
        return parseAccountRaw2 (jvParams, jss::destination_account);
    }

    // channel_authorize <private_key> <channel_id> <drops>
    Json::Value parseChannelAuthorize (Json::Value const& jvParams)
    {
        Json::Value jvRequest (Json::objectValue);

        jvRequest[jss::secret] = jvParams[0u];
        {
            // verify the channel id is a valid 256 bit number
            uint256 channelId;
            if (!channelId.SetHexExact (jvParams[1u].asString ()))
                return rpcError (rpcCHANNEL_MALFORMED);
        }
        jvRequest[jss::channel_id] = jvParams[1u].asString ();

        try
        {
            auto const drops = std::stoul (jvParams[2u].asString ());
            (void)drops;  // just used for error checking
            jvRequest[jss::amount] = jvParams[2u];
        }
        catch (std::exception const&)
        {
            return rpcError (rpcCHANNEL_AMT_MALFORMED);
        }

        return jvRequest;
    }

    // channel_verify <public_key> <channel_id> <drops> <signature>
    Json::Value parseChannelVerify (Json::Value const& jvParams)
    {
        std::string const strPk = jvParams[0u].asString ();

        if (!parseBase58<PublicKey> (TokenType::TOKEN_ACCOUNT_PUBLIC, strPk))
            return rpcError (rpcPUBLIC_MALFORMED);

        Json::Value jvRequest (Json::objectValue);

        jvRequest[jss::public_key] = strPk;
        {
            // verify the channel id is a valid 256 bit number
            uint256 channelId;
            if (!channelId.SetHexExact (jvParams[1u].asString ()))
                return rpcError (rpcCHANNEL_MALFORMED);
        }
        jvRequest[jss::channel_id] = jvParams[1u].asString ();
        try
        {
            auto const drops = std::stoul (jvParams[2u].asString ());
            (void)drops;  // just used for error checking
            jvRequest[jss::amount] = jvParams[2u];
        }
        catch (std::exception const&)
        {
            return rpcError (rpcCHANNEL_AMT_MALFORMED);
        }
        jvRequest[jss::signature] = jvParams[3u].asString ();

        return jvRequest;
    }

    Json::Value parseAccountRaw2 (Json::Value const& jvParams,
                                  char const * const acc2Field)
    {
        std::array<char const* const, 2> accFields{{jss::account, acc2Field}};
        auto const nParams = jvParams.size ();
        Json::Value jvRequest (Json::objectValue);
        for (auto i = 0; i < nParams; ++i)
        {
            std::string strParam = jvParams[i].asString ();

            if (i==1 && strParam.empty())
                continue;

            // Parameters 0 and 1 are accounts
            if (i < 2)
            {
                if (parseBase58<PublicKey> (
                        TokenType::TOKEN_ACCOUNT_PUBLIC, strParam) ||
                    parseBase58<AccountID> (strParam) ||
                    parseGenericSeed (strParam))
                {
                    jvRequest[accFields[i]] = std::move (strParam);
                }
                else
                {
                    return rpcError (rpcACT_MALFORMED);
                }
            }
            else
            {
                if (jvParseLedger (jvRequest, strParam))
                    return jvRequest;
                return rpcError (rpcLGR_IDX_MALFORMED);
            }
        }

        return jvRequest;
    }

    // TODO: Get index from an alternate syntax: rXYZ:<index>
    Json::Value parseAccountRaw1 (Json::Value const& jvParams)
    {
        std::string     strIdent    = jvParams[0u].asString ();
        unsigned int    iCursor     = jvParams.size ();
        bool            bStrict     = false;

        if (iCursor >= 2 && jvParams[iCursor - 1] == jss::strict)
        {
            bStrict = true;
            --iCursor;
        }

        if (! parseBase58<PublicKey>(TokenType::TOKEN_ACCOUNT_PUBLIC, strIdent) &&
            ! parseBase58<AccountID>(strIdent) &&
            ! parseGenericSeed(strIdent))
            return rpcError (rpcACT_MALFORMED);

        // Get info on account.
        Json::Value jvRequest (Json::objectValue);

        jvRequest[jss::account]    = strIdent;

        if (bStrict)
            jvRequest[jss::strict]     = 1;

        if (iCursor == 2 && !jvParseLedger (jvRequest, jvParams[1u].asString ()))
            return rpcError (rpcLGR_IDX_MALFORMED);

        return jvRequest;
    }

    //_path_find <json> [<ledger>]
    Json::Value parseMTChainPathFind (Json::Value const& jvParams)
    {
        Json::Reader    reader;
        Json::Value     jvRequest;
        bool            bLedger     = 2 == jvParams.size ();

        JLOG (j_.trace()) << "RPC json: " << jvParams[0u];

        if (reader.parse (jvParams[0u].asString (), jvRequest))
        {
            if (bLedger)
            {
                jvParseLedger (jvRequest, jvParams[1u].asString ());
            }

            return jvRequest;
        }

        return rpcError (rpcINVALID_PARAMS);
    }

    bool hexCodeString (const char *key, Json::Value &val)
    {
        if (val.isString())
        {
            auto const& field = SField::getField (key);
            if (field == sfInvalid)
                return false;

            if (field.fieldType != STI_VL)
                return false;

            auto const& str = val.asString();
            if (!strUnHex(str).second)
            {
                val = strHex(str);
                return true;
            }

            return false;
        }

        if (val.isArray())
        {
            auto ret = false;
            for (auto &e : val)
            {
                ret |= hexCodeString("", e);
            }

            return ret;
        }

        if (val.isObject())
        {
            auto const& field = SField::getField (key);
            if (field != sfInvalid && field.fieldType == STI_VL)
            {
                val = strHex(to_string(val));
                return true;
            }

            auto ret = false;
            for (auto itr = val.begin(); itr != val.end(); ++itr)
            {
                ret |= hexCodeString(itr.memberName(), *itr);
            }

            return ret;
        }

        return false;
    }

    // sign/submit any transaction to the network
    //
    // sign <private_key> <json> offline
    // submit <private_key> <json>
    // submit <tx_blob>
    Json::Value parseSignSubmit (Json::Value const& jvParams)
    {
        Json::Value     txJSON;
        Json::Reader    reader;
        bool const      bOffline    = 3 == jvParams.size () && jvParams[2u].asString () == "offline";

        if (1 == jvParams.size ())
        {
            // Submitting tx_blob

            Json::Value jvRequest;

            jvRequest[jss::tx_blob]    = jvParams[0u].asString ();

            return jvRequest;
        }
        else if ((2 == jvParams.size () || bOffline)
                 && reader.parse (jvParams[1u].asString (), txJSON))
        {
            // Signing or submitting tx_json.
            Json::Value jvRequest;

            jvRequest[jss::secret]     = jvParams[0u].asString ();
            hexCodeString("", txJSON);
            jvRequest[jss::tx_json]    = txJSON;

            if (bOffline)
                jvRequest[jss::offline]    = true;

            return jvRequest;
        }
        else if (jvParams.size() == 3 && reader.parse (jvParams[1u].asString (), txJSON))
        {
            Json::Value jvRequest;

            jvRequest[jss::passphrase]     = jvParams[0u].asString ();
            jvRequest[jss::tx_json]        = txJSON;
            jvRequest[jss::key_type]       = jvParams[2u].asString ();

            return jvRequest;
        }

        return rpcError (rpcINVALID_PARAMS);
    }

	// submit smart contract to the network
	//
	// sc <hash_id> <private_key> <json>
	Json::Value parseSc(Json::Value const& jvParams)
	{
		Json::Value     txJSON;
		Json::Reader    reader;

		if (1 == jvParams.size())
		{
			Json::Value jvRequest;

			jvRequest["transaction"] = jvParams[0u].asString();

			return jvRequest;
		}
		else if ((3 == jvParams.size())
			&& reader.parse(jvParams[2u].asString(), txJSON))
		{
			// Signing or submitting tx_json.
			Json::Value jvRequest;

			jvRequest["transaction"] = jvParams[0u].asString();
			jvRequest[jss::secret] = jvParams[1u].asString();
			jvRequest[jss::tx_json] = txJSON;

			return jvRequest;
		}

		return rpcError(rpcINVALID_PARAMS);
	}

    // submit any multisigned transaction to the network
    //
    // submit_multisigned <json>
    Json::Value parseSubmitMultiSigned (Json::Value const& jvParams)
    {
        if (1 == jvParams.size ())
        {
            Json::Value     txJSON;
            Json::Reader    reader;
            if (reader.parse (jvParams[0u].asString (), txJSON))
            {
                Json::Value jvRequest;
                jvRequest[jss::tx_json] = txJSON;
                return jvRequest;
            }
        }

        return rpcError (rpcINVALID_PARAMS);
    }

    // tx <transaction_id>
    Json::Value parseTx (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        if (jvParams.size () > 1)
        {
            if (jvParams[1u].asString () == jss::binary)
                jvRequest[jss::binary] = true;
        }

        jvRequest["transaction"]    = jvParams[0u].asString ();
        return jvRequest;
    }

    Json::Value parseFileDownload (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        auto txnID = jvParams[0u].asString();
        if (!isHex64 (txnID))
        {
            return rpcError (rpcINVALID_PARAMS);
        }

        auto passwd = jvParams[1u].asString();
        if (!isHex64 (passwd))
        {
            return rpcError (rpcINVALID_PARAMS);
        }

        jvRequest[jss::transaction] = txnID;
        jvRequest[jss::password] = passwd;
        jvRequest[jss::secret] = jvParams[2u].asString();

        auto keypair = RPC::keypairForSignature(jvRequest, jvRequest);
        if (!RPC::contains_error(jvRequest))
        {
            auto m = txnID + passwd;
            jvRequest.removeMember(jss::secret);
            auto sign = mtchain::sign(keypair.first, keypair.second, makeSlice(m));
            jvRequest[jss::signature] = strHex(sign);
        }

        return jvRequest;
    }

    Json::Value parseFileUpload (Json::Value const& jvParams)
    {
#ifdef IPFS_ENABLE
        Json::Value jvRequest;

        auto filepath = jvParams[1u].asString();
        boost::filesystem::path path = filepath;
        auto filename = path.leaf().string();
        auto ip = config_.rpc_ip ? config_.rpc_ip->to_string() : "localhost";
        ipfs::Json result;
        ipfs::Client client(ip, jvParams.size() < 3 ? 19191 : jvParams[2].asUInt());

        try {
            client.FilesAdd ({
                    {filename , ipfs::http::FileUpload::Type::kFileName, filepath }
                }, &result);
            JLOG (j_.warn()) << "upload file ok: \n" << result.dump(4) << "\n";
        } catch (std::exception const& e)
        {
            JLOG (j_.error()) << e.what() << "\n";
            return RPC::make_error (rpcINTERNAL, "Upload file " + filename + " failed");
        }

        jvRequest[jss::secret] = jvParams[0u].asString();
        auto keypair = RPC::keypairForSignature(jvRequest, jvRequest);
        if (RPC::contains_error(jvRequest))
        {
            return rpcError (rpcINVALID_PARAMS);
        }

        Json::Reader reader;
        Json::Value jvResult;
        if (!reader.parse (result.dump(0), jvResult))
        {
            return rpcError (rpcINVALID_PARAMS);
        }

        Json::Value txJSON = Json::objectValue;
        txJSON[jss::TransactionType] = "Payment";
        txJSON[jss::Account] = toBase58(calcAccountID(keypair.first));

        if (jvResult.isMember(jss::accountID))
        {
            txJSON[jss::Destination] = jvResult[jss::accountID];
        }
        else if (jvResult.isMember(jss::charge_account))
        {
            txJSON[jss::Destination] = jvResult[jss::charge_account];
        }
        else
        {
            return rpcError (rpcINTERNAL);
        }

        std::string fee;
        if (jvResult.isMember(jss::storageFee))
        {
            fee = "1/" + jvResult[jss::storageFee].asString();
        }
        else if (jvResult.isMember(jss::fee))
        {
            fee = jvResult[jss::fee].asString();
        }
        else
        {
            return rpcError (rpcINTERNAL);
        }

        auto fileSize = jvResult[jss::files][0U][jss::size].asUInt();
        txJSON[jss::Amount] = multiply(parse_fraction(fee), fileSize, mIssue()).getText();

        Json::Value memo = Json::objectValue;
        memo[jss::MemoType] = "FileUpload";
        memo[jss::MemoFormat] = "JSON";
        memo[jss::MemoData] = jvResult;

        Json::Value objMemo = Json::objectValue;
        objMemo[jss::Memo] = memo;

        Json::Value memos = Json::arrayValue;
        memos.append(objMemo);

        txJSON[jss::Memos] = memos;
        //JLOG (j_.warn()) << to_string(txJSON);

        jvRequest = Json::arrayValue;
        jvRequest[0U] = jvParams[0u].asString();
        jvRequest[1U] = to_string(txJSON);

        jvRequest = parseSignSubmit(jvRequest);
        jvRequest[jss::method] = "submit";
        //JLOG (j_.warn()) << to_string(jvRequest);

        return jvRequest;
#else
        return rpcError (rpcNOT_SUPPORTED);
#endif
    }

    // tx_history <index>
    Json::Value parseTxHistory (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        jvRequest[jss::start]  = jvParams[0u].asUInt ();

        return jvRequest;
    }

    // validation_create [<pass_phrase>|<seed>|<seed_key>]
    //
    // NOTE: It is poor security to specify secret information on the command line.  This information might be saved in the command
    // shell history file (e.g. .bash_history) and it may be leaked via the process status command (i.e. ps).
    Json::Value parseValidationCreate (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        if (jvParams.size ())
            jvRequest[jss::secret]     = jvParams[0u].asString ();

        return jvRequest;
    }

    // validation_seed [<pass_phrase>|<seed>|<seed_key>]
    //
    // NOTE: It is poor security to specify secret information on the command line.  This information might be saved in the command
    // shell history file (e.g. .bash_history) and it may be leaked via the process status command (i.e. ps).
    Json::Value parseValidationSeed (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        if (jvParams.size ())
            jvRequest[jss::secret]     = jvParams[0u].asString ();

        return jvRequest;
    }

    // wallet_propose [<passphrase>]
    // <passphrase> is only for testing. Master seeds should only be generated randomly.
    Json::Value parseWalletPropose (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        if (jvParams.size ())
        {
            jvRequest[jss::key_type]   = jvParams[0u].asString ();
            if (jvParams.size() > 1)
                jvRequest[jss::passphrase]   = jvParams[1u].asString ();
        }

        return jvRequest;
    }

    // wallet_seed [<seed>|<passphrase>|<passkey>]
    Json::Value parseWalletSeed (Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        if (jvParams.size ())
            jvRequest[jss::secret]     = jvParams[0u].asString ();

        return jvRequest;
    }

    // parse gateway balances
    // gateway_balances [<ledger>] <issuer_account> [ <hotwallet> [ <hotwallet> ]]

    Json::Value parseGatewayBalances (Json::Value const& jvParams)
    {
        unsigned int index = 0;
        const unsigned int size = jvParams.size ();

        Json::Value jvRequest;

        std::string param = jvParams[index++].asString ();
        if (param.empty ())
            return RPC::make_param_error ("Invalid first parameter");

        if (param[0] != 'r')
        {
            if (param.size() == 64)
                jvRequest[jss::ledger_hash] = param;
            else
                jvRequest[jss::ledger_index] = param;

            if (size <= index)
                return RPC::make_param_error ("Invalid hotwallet");

            param = jvParams[index++].asString ();
        }

        jvRequest[jss::account] = param;

        if (index < size)
        {
            Json::Value& hotWallets =
                (jvRequest["hotwallet"] = Json::arrayValue);
            while (index < size)
                hotWallets.append (jvParams[index++].asString ());
        }

        return jvRequest;
    }

    // ledger_entry <index> [binary]
    // <index> is the key of a ledger entry which can be got by ledger_entry_index command
    Json::Value parseLedgerEntry(Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        jvRequest[jss::index] = jvParams[0u];
        jvRequest[jss::binary] = jvParams.size() > 1 && jvParams[1u].asString() == "binary";

        return jvRequest;
    }

    // ledger_entry_index <LedgerEntryType> <other params decided by LedgerEntryType>
    Json::Value parseLedgerEntryIndex(Json::Value const& jvParams)
    {
        Json::Value jvRequest(Json::objectValue);
        Json::Value jvArgs(Json::arrayValue);

        for (auto i = 1; i < jvParams.size(); ++i) {
            jvArgs.append(jvParams[i]);
        }

        jvRequest[sfLedgerEntryType.getJsonName()] = jvParams[0u];
        jvRequest[jss::args] = jvArgs;
        return jvRequest;
    }

    Json::Value parseTokenInfo(Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        switch (jvParams.size())
        {
        case 1:
            jvRequest[sfTokenID.getJsonName()] = jvParams[0u];
            break;
        case 2:
            jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
            std::uint64_t index;
            if (beast::lexicalCastChecked(index, jvParams[1u].asString()))
            {
                jvRequest[jss::index] = jvParams[1u];
            }
            else
            {
                jvRequest[jss::id] = jvParams[1u];
            }
            break;
        default:
            jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
            jvRequest[sfOwner.getJsonName()] = jvParams[1u];
            jvRequest[jss::index] = jvParams[2u];
        }

        return jvRequest;
    }

    Json::Value parseAssetBalance(Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        jvRequest[jss::Account] = jvParams[0u];
        jvRequest[sfAssetID.getJsonName()] = jvParams[1u];

        return jvRequest;
    }

    Json::Value parseAssetInfo(Json::Value const& jvParams)
    {
        Json::Value jvRequest;;

        if (jvParams.size() < 2)
        {
            jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
        }
        else
        {
            jvRequest[jss::Account] = jvParams[0u];
            jvRequest[jss::index] = jvParams[1u];
        }

        return jvRequest;
    }

    Json::Value parseAssetOperator(Json::Value const& jvParams)
    {
        Json::Value jvRequest;

        jvRequest[jss::Account] = jvParams[0u];
        jvRequest[sfAssetID.getJsonName()] = jvParams[1u];
        jvRequest[sfOwner.getJsonName()] = jvParams[2u];

        return jvRequest;
    }
    Json::Value parseAssetAllTokenInfo(Json::Value const& jvParams)    
    {
       Json::Value jvRequest;
       if(jvParams.size() == 1)
       {
               jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
       }
       if(jvParams.size() == 2)
       {
               jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
               jvRequest[jss::limit] = jvParams[1u].asInt();
       }
       if(jvParams.size() == 3)
       {
               jvRequest[sfAssetID.getJsonName()] = jvParams[1u];
               jvRequest[jss::limit] = jvParams[1u].asInt();
	       jvRequest[jss::marker] = jvParams[2u].asInt();
       }
       return jvRequest;       
    }

    Json::Value parseAccountAllAssetInfo(Json::Value const& jvParams)
    {
       Json::Value jvRequest;
       if(jvParams.size() == 1)
       {
               jvRequest[jss::Account] = jvParams[0u];
       } 
       if(jvParams.size() == 2)
       {
               jvRequest[jss::Account] = jvParams[0u];
       		if(!jvParams[1u].isObject())
		{
			jvRequest[jss::limit] = jvParams[1u].asInt();
		}
		else
		{
			jvRequest[jss::marker] = jvParams[1u].asInt();
		}
       }
       if(jvParams.size() == 3)
       {
               jvRequest[sfAssetID.getJsonName()] = jvParams[0u];
               jvRequest[jss::limit] = jvParams[1u].asInt();
	       jvRequest[jss::marker] = jvParams[2u].asInt();
       }
       return jvRequest;       
    }
	
    Json::Value parseAccountAllTokenInfo(Json::Value const& jvParams)
    {
	Json::Value jvRequest;

       if(jvParams.size() == 1)
       {
               jvRequest[jss::Account] = jvParams[0u];
       } 
       if(jvParams.size() == 2)
       {
               jvRequest[jss::Account] = jvParams[0u];
		if(!jvParams[1u].isObject())
		{
       			jvRequest[jss::limit] = jvParams[1u].asInt();
		}
		else
		{
			jvRequest[jss::marker] = jvParams[1u];
		}
       }
       if(jvParams.size() == 3)
       {
       //        jvRequest[sfAssetID.getJsonName()] = jvParams[1u];
               jvRequest[jss::Account] = jvParams[0u];
               jvRequest[jss::limit] = jvParams[1u].asInt();
		jvRequest[jss::marker] = jvParams[2u];
       }
       return jvRequest;
    }
public:
    //--------------------------------------------------------------------------

    explicit
    RPCParser (beast::Journal j, Config const& config)
        :j_ (j), config_(config) {}

    //--------------------------------------------------------------------------

    // Convert a rpc method and params to a request.
    // <-- { method: xyz, params: [... ] } or { error: ..., ... }
    Json::Value parseCommand (std::string strMethod, Json::Value jvParams, bool allowAnyCommand)
    {
        if (auto stream = j_.trace())
        {
            stream << "Method: '" << strMethod << "'";
            stream << "Params: " << jvParams;
        }

        struct Command
        {
            const char*     name;
            parseFuncPtr    parse;
            int             minParams;
            int             maxParams;
        };

        // FIXME: replace this with a function-static std::map and the lookup
        // code with std::map::find when the problem with magic statics on
        // Visual Studio is fixed.
        static
        Command const commands[] =
        {
            // Request-response methods
            // - Returns an error, or the request.
            // - To modify the method, provide a new method in the request.
            {   "account_currencies",   &RPCParser::parseAccountCurrencies,     1,  2   },
            {   "account_info",         &RPCParser::parseAccountItems,          1,  2   },
            {   "account_lines",        &RPCParser::parseAccountLines,          1,  5   },
            {   "account_channels",     &RPCParser::parseAccountChannels,       1,  3   },
            {   "account_objects",      &RPCParser::parseAccountItems,          1,  5   },
            {   "account_offers",       &RPCParser::parseAccountItems,          1,  4   },
            {   "account_tx",           &RPCParser::parseAccountTransactions,   1,  8   },
            {   "book_offers",          &RPCParser::parseBookOffers,            2,  7   },
            {   "can_delete",           &RPCParser::parseCanDelete,             0,  1   },
            {   "channel_authorize",    &RPCParser::parseChannelAuthorize,      3,  3   },
            {   "channel_verify",       &RPCParser::parseChannelVerify,         4,  4   },
            {   "connect",              &RPCParser::parseConnect,               1,  2   },
            {   "consensus_info",       &RPCParser::parseAsIs,                  0,  0   },
            {   "feature",              &RPCParser::parseFeature,               0,  2   },
            {   "fetch_info",           &RPCParser::parseFetchInfo,             0,  1   },
            {   "gateway_balances",     &RPCParser::parseGatewayBalances  ,     1,  -1  },
            {   "get_counts",           &RPCParser::parseGetCounts,             0,  1   },
            {   "ipfs_fee_info",        &RPCParser::parseIpfsFeeInfo,           0,  0   },
            {   "json",                 &RPCParser::parseJson,                  2,  2   },
            {   "json2",                &RPCParser::parseJson2,                 1,  1   },
            {   "ledger",               &RPCParser::parseLedger,                0,  2   },
            {   "ledger_accept",        &RPCParser::parseAsIs,                  0,  0   },
            {   "ledger_closed",        &RPCParser::parseAsIs,                  0,  0   },
            {   "ledger_current",       &RPCParser::parseAsIs,                  0,  0   },
            {   "ledger_entry",         &RPCParser::parseLedgerEntry,           1,  2   },
            {   "ledger_entry_index",   &RPCParser::parseLedgerEntryIndex,      2,  4   },
            {   "ledger_header",        &RPCParser::parseLedgerId,              1,  1   },
            {   "ledger_request",       &RPCParser::parseLedgerId,              1,  1   },
            {   "log_level",            &RPCParser::parseLogLevel,              0,  2   },
            {   "logrotate",            &RPCParser::parseAsIs,                  0,  0   },
            {   "owner_info",           &RPCParser::parseAccountItems,          1,  2   },
            {   "peers",                &RPCParser::parseAsIs,                  0,  0   },
            {   "ping",                 &RPCParser::parseAsIs,                  0,  0   },
            {   "print",                &RPCParser::parseAsIs,                  0,  1   },
    //      {   "profile",              &RPCParser::parseProfile,               1,  9   },
            {   "random",               &RPCParser::parseAsIs,                  0,  0   },
            {   "mtchain_path_find",    &RPCParser::parseMTChainPathFind,       1,  2   },
            {   "sign",                 &RPCParser::parseSignSubmit,            2,  3   },
            {   "sign_for",             &RPCParser::parseSignFor,               3,  4   },
            {   "submit",               &RPCParser::parseSignSubmit,            1,  3   },
            {   "submit_multisigned",   &RPCParser::parseSubmitMultiSigned,     1,  1   },
            {   "server_info",          &RPCParser::parseAsIs,                  0,  0   },
            {   "server_state",         &RPCParser::parseAsIs,                  0,  0   },
            {   "stop",                 &RPCParser::parseAsIs,                  0,  0   },
    //      {   "transaction_entry",    &RPCParser::parseTransactionEntry,     -1,  -1  },
            {   "tx",                   &RPCParser::parseTx,                    1,  2   },
            {   "sc",                   &RPCParser::parseSc,                    1,  4   },
            {   "download",             &RPCParser::parseFileDownload,          3,  3   },
            {   "upload",               &RPCParser::parseFileUpload,            2,  3   },
            {   "tx_account",           &RPCParser::parseTxAccount,             1,  7   },
            {   "tx_history",           &RPCParser::parseTxHistory,             1,  1   },
            {   "unl_list",             &RPCParser::parseAsIs,                  0,  0   },
            {   "validation_create",    &RPCParser::parseValidationCreate,      0,  1   },
            {   "validation_seed",      &RPCParser::parseValidationSeed,        0,  1   },
            {   "version",              &RPCParser::parseAsIs,                  0,  0   },
            {   "wallet_propose",       &RPCParser::parseWalletPropose,         0,  2   },
            {   "wallet_seed",          &RPCParser::parseWalletSeed,            0,  1   },
            {   "internal",             &RPCParser::parseInternal,              1,  -1  },

            // Evented methods
            {   "path_find",            &RPCParser::parseEvented,               -1, -1  },
            {   "subscribe",            &RPCParser::parseEvented,               -1, -1  },
            {   "unsubscribe",          &RPCParser::parseEvented,               -1, -1  },
            {   "get_token_info",       &RPCParser::parseTokenInfo,              1,  3  },
            {   "get_asset_balance",    &RPCParser::parseAssetBalance,           2,  2  },
            {   "get_asset_info",       &RPCParser::parseAssetInfo,              1,  2  },
            {   "is_asset_operator",    &RPCParser::parseAssetOperator,          3,  3  },
            {   "get_asset_all_token_info", &RPCParser::parseAssetAllTokenInfo,         1,  3  },
            {   "get_account_all_asset_info", &RPCParser::parseAccountAllAssetInfo,         1,  3  },
            {   "get_account_all_token_info", &RPCParser::parseAccountAllTokenInfo,         1,  3  },
        };

        auto const count = jvParams.size ();

        for (auto const& command : commands)
        {
            if (strMethod == command.name)
            {
                if ((command.minParams >= 0 && count < command.minParams) ||
                    (command.maxParams >= 0 && count > command.maxParams))
                {
                    JLOG (j_.debug()) <<
                        "Wrong number of parameters for " << command.name <<
                        " minimum=" << command.minParams <<
                        " maximum=" << command.maxParams <<
                        " actual=" << count;

                    return rpcError (rpcBAD_SYNTAX);
                }

                return (this->* (command.parse)) (jvParams);
            }
        }

        // The command could not be found
        if (!allowAnyCommand)
            return rpcError (rpcUNKNOWN_COMMAND);

        return parseAsIs (jvParams);
    }
};

//------------------------------------------------------------------------------

//
// JSON-RPC protocol.  Bitcoin speaks version 1.0 for maximum compatibility,
// but uses JSON-RPC 1.1/2.0 standards for parts of the 1.0 standard that were
// unspecified (HTTP errors and contents of 'error').
//
// 1.0 spec: http://json-rpc.org/wiki/specification
// 1.2 spec: http://groups.google.com/group/json-rpc/web/json-rpc-over-http
//

std::string JSONRPCRequest (std::string const& strMethod, Json::Value const& params, Json::Value const& id)
{
    Json::Value request;
    request[jss::method] = strMethod;
    request[jss::params] = params;
    request[jss::id] = id;
    return to_string (request) + "\n";
}

bool unHexCodeString (const char *key, Json::Value &val)
{
    if (val.isString())
    {
        auto const& field = SField::getField (key);
        if (field == sfInvalid)
            return false;

        if (field.fieldType != STI_VL)
            return false;

        std::pair<Blob, bool> uh = strUnHex(val.asString());
        if (uh.second)
        {
            auto str = to_string(uh.first);
            if (std::find_if_not(str.begin(), str.end(), isprint) == str.end())
            {
                Json::Reader reader;
                Json::Value obj;
                val = reader.parse(str, obj) ? obj : str;
                return true;
            }
        }

        return false;
    }

    if (val.isArray())
    {
        auto ret = false;
        for (auto &e : val)
        {
            ret |= unHexCodeString("", e);
        }

        return ret;
    }

    if (val.isObject())
    {
        auto ret = false;
        for (auto itr = val.begin(); itr != val.end(); ++itr)
        {
            ret |= unHexCodeString(itr.memberName(), *itr);
        }

        return ret;
    }

    return false;
}

struct RPCCallImp
{
    // VFALCO NOTE Is this a to-do comment or a doc comment?
    // Place the async result somewhere useful.
    static void callRPCHandler (Json::Value* jvOutput, Json::Value const& jvInput)
    {
        (*jvOutput) = jvInput;
    }

    static bool onResponse (
        std::function<void (Json::Value const& jvInput)> callbackFuncP,
            const boost::system::error_code& ecResult, int iStatus,
	std::string const& strData, beast::Journal j,  bool output)
    {
        if (callbackFuncP)
        {
            // Only care about the result, if we care to deliver it callbackFuncP.

            // Receive reply
            if (iStatus == 401)
                Throw<std::runtime_error> (
                    "incorrect rpcuser or rpcpassword (authorization failed)");
            else if ((iStatus >= 400) && (iStatus != 400) && (iStatus != 404) && (iStatus != 500)) // ?
                Throw<std::runtime_error> (
                    std::string ("server returned HTTP error ") +
                        std::to_string (iStatus));
            else if (strData.empty ())
                Throw<std::runtime_error> ("no response from server");

            // Parse reply
            JLOG (j.debug()) << "RPC reply: " << strData << std::endl;

            Json::Reader    reader;
            Json::Value     jvReply;

            if (!reader.parse (strData, jvReply) ||
                !(jvReply.isMember(jss::result) && jvReply[jss::result].isMember(jss::status)))
	    {
                if (output)
                {
                    std::cout.write (strData.c_str(), strData.size());
                }
                Throw<std::runtime_error> ("couldn't parse reply from server");
	    }

            if (!jvReply)
                Throw<std::runtime_error> ("expected reply to have result, error and id properties");

            // 增加STI_VL类型字段编码字符串或JSON Object时的可读性
            if (jvReply[jss::result][jss::status] == "success")
            {
                boost::optional<Json::Value> txJSON;
                if (jvReply[jss::result].isMember(jss::tx_json))
                {
                    txJSON = jvReply[jss::result][jss::tx_json];
                }

                if (unHexCodeString("", jvReply))
                {
                    if (txJSON)
                    {
                        jvReply[jss::result]["tx_json_human"] = jvReply[jss::result][jss::tx_json];
                        jvReply[jss::result][jss::tx_json] = *txJSON;
                    }
                }
            }

            Json::Value     jvResult (Json::objectValue);
            jvResult["result"] = jvReply;

            (callbackFuncP) (jvResult);
        }

        return false;
    }

    // Build the request.
    static void onRequest (std::string const& strMethod, Json::Value const& jvParams,
        const std::map<std::string, std::string>& mHeaders, std::string const& strPath,
            boost::asio::streambuf& sb, std::string const& strHost, beast::Journal j)
    {
        JLOG (j.debug()) << "requestRPC: strPath='" << strPath << "'";

        std::ostream    osRequest (&sb);
        osRequest <<
                  createHTTPPost (
                      strHost,
                      strPath,
                      JSONRPCRequest (strMethod, jvParams, Json::Value (1)),
                      mHeaders);
    }
};

//------------------------------------------------------------------------------

// Used internally by rpcClient.
static Json::Value
rpcCmdLineToJson (std::vector<std::string> const& args, Json::Value& retParams,
                  beast::Journal j, Config const& config)
{
    Json::Value jvRequest (Json::objectValue);

    RPCParser   rpParser (j, config);
    Json::Value jvRpcParams (Json::arrayValue);

    for (int i = 1; i != args.size (); i++)
        jvRpcParams.append (args[i]);

    retParams = Json::Value (Json::objectValue);

    retParams[jss::method] = args[0];
    retParams[jss::params] = jvRpcParams;

    jvRequest   = rpParser.parseCommand (args[0], jvRpcParams, true);

    JLOG (j.trace()) << "RPC Request: " << jvRequest << std::endl;

    return jvRequest;
}

Json::Value
cmdLineToJSONRPC (std::vector<std::string> const& args, beast::Journal j)
{
    Json::Value jv = Json::Value (Json::objectValue);
    auto const paramsObj = rpcCmdLineToJson (args, jv, j, Config());

    // Re-use jv to return our formatted result.
    jv.clear();

    // Allow parser to rewrite method.
    jv[jss::method] = paramsObj.isMember (jss::method) ?
        paramsObj[jss::method].asString() : args[0];

    // If paramsObj is not empty, put it in a [params] array.
    if (paramsObj.begin() != paramsObj.end())
    {
        auto& paramsArray = Json::setArray (jv, jss::params);
        paramsArray.append (paramsObj);
    }
    if (paramsObj.isMember(jss::jsonrpc))
        jv[jss::jsonrpc] = paramsObj[jss::jsonrpc];
    if (paramsObj.isMember(jss::FinPalrpc))
        jv[jss::FinPalrpc] = paramsObj[jss::FinPalrpc];
    if (paramsObj.isMember(jss::id))
        jv[jss::id] = paramsObj[jss::id];
    return jv;
}

//------------------------------------------------------------------------------
std::pair<int, Json::Value>
rpcClient(std::vector<std::string> const& args,
    Config const& config, Logs& logs)
{
    static_assert(rpcBAD_SYNTAX == 1 && rpcSUCCESS == 0,
        "Expect specific rpc enum values.");
    if (args.empty ())
        return { rpcBAD_SYNTAX, {} }; // rpcBAD_SYNTAX = print usage

    int         nRet = rpcSUCCESS;
    Json::Value jvOutput;
    Json::Value jvRequest (Json::objectValue);
    std::string strMethod;
    try
    {
        Json::Value jvRpc   = Json::Value (Json::objectValue);
        jvRequest = rpcCmdLineToJson (args, jvRpc, logs.journal ("RPCParser"), config);

        if (jvRequest.isMember (jss::error))
        {
            jvOutput            = jvRequest;
            jvOutput["rpc"]     = jvRpc;
        }
        else
        {
            ServerHandler::Setup setup;
            try
            {
                setup = setup_ServerHandler(
                    config,
                    beast::logstream { logs.journal ("HTTPClient").warn() });
            }
            catch (std::exception const&)
            {
                // ignore any exceptions, so the command
                // line client works without a config file
            }

            if (config.rpc_ip)
                setup.client.ip = config.rpc_ip->to_string();
            if (config.rpc_port)
                setup.client.port = *config.rpc_port;

            Json::Value jvParams (Json::arrayValue);

            if (!setup.client.admin_user.empty ())
                jvRequest["admin_user"] = setup.client.admin_user;

            if (!setup.client.admin_password.empty ())
                jvRequest["admin_password"] = setup.client.admin_password;

            jvParams.append (jvRequest);

            if (jvRequest.isMember(jss::params))
            {
                auto const& params = jvRequest[jss::params];
                assert(params.size() == 1);
                jvParams.append(params[0u]);
            }

            {
                // Allow parser to rewrite method.
                strMethod = jvRequest.isMember (jss::method) ? jvRequest[jss::method].asString ()
                    : args[0];

                boost::asio::io_service isService;
                RPCCall::fromNetwork (
                    isService,
                    setup.client.ip,
                    setup.client.port,
                    setup.client.user,
                    setup.client.password,
                    "",
                    strMethod,
                    jvParams,                               // Parsed, execute.
                    setup.client.secure != 0,                // Use SSL
                    config.quiet(),
                    logs,
                    std::bind (RPCCallImp::callRPCHandler, &jvOutput,
                               std::placeholders::_1));
                isService.run(); // This blocks until there is no more outstanding async calls.
            }

            if (jvOutput.isMember ("result"))
            {
                // Had a successful JSON-RPC 2.0 call.
                jvOutput    = jvOutput["result"];

                // jvOutput may report a server side error.
                // It should report "status".
            }
            else
            {
                // Transport error.
                Json::Value jvRpcError  = jvOutput;

                jvOutput            = rpcError (rpcJSON_RPC);
                jvOutput["result"]  = jvRpcError;
            }

            // If had an error, supply invocation in result.
            if (jvOutput.isMember (jss::error))
            {
                jvOutput["rpc"]             = jvRpc;            // How the command was seen as method + params.
                jvOutput["request_sent"]    = jvRequest;        // How the command was translated.
            }
        }

        if (jvOutput.isMember (jss::error))
        {
            jvOutput[jss::status]  = "error";

            nRet    = jvOutput.isMember (jss::error_code)
                      ? beast::lexicalCast <int> (jvOutput[jss::error_code].asString ())
                      : rpcBAD_SYNTAX;
        }

        // YYY We could have a command line flag for single line output for scripts.
        // YYY We would intercept output here and simplify it.
    }
    catch (std::exception& e)
    {
        if (strMethod == "download")
        {
            return { rpcBAD_SYNTAX, std::move(jvOutput) };
        }

        jvOutput                = rpcError (rpcINTERNAL);
        jvOutput["error_what"]  = e.what ();
        nRet                    = rpcINTERNAL;
    }

    if (jvRequest.isMember(jss::jsonrpc))
        jvOutput[jss::jsonrpc] = jvRequest[jss::jsonrpc];
    if (jvRequest.isMember(jss::FinPalrpc))
        jvOutput[jss::FinPalrpc] = jvRequest[jss::FinPalrpc];
    if (jvRequest.isMember(jss::id))
        jvOutput[jss::id] = jvRequest[jss::id];

    return { nRet, std::move(jvOutput) };
}

//------------------------------------------------------------------------------

namespace RPCCall {

int fromCommandLine (
    Config const& config,
    const std::vector<std::string>& vCmd,
    Logs& logs)
{
    auto const result = rpcClient(vCmd, config, logs);

    if (result.first != rpcBAD_SYNTAX)
        std::cout << result.second.toStyledString ();

    return result.first;
}

//------------------------------------------------------------------------------

void fromNetwork (
    boost::asio::io_service& io_service,
    std::string const& strIp, const std::uint16_t iPort,
    std::string const& strUsername, std::string const& strPassword,
    std::string const& strPath, std::string const& strMethod,
    Json::Value const& jvParams, const bool bSSL, const bool quiet,
    Logs& logs,
    std::function<void (Json::Value const& jvInput)> callbackFuncP)
{
    auto j = logs.journal ("HTTPClient");

    // Connect to localhost
    if (!quiet)
    {
        JLOG(j.info()) << (bSSL ? "Securely connecting to " : "Connecting to ") <<
            strIp << ":" << iPort << std::endl;
    }

    // HTTP basic authentication
    auto const auth = beast::detail::base64_encode(strUsername + ":" + strPassword);

    std::map<std::string, std::string> mapRequestHeaders;

    mapRequestHeaders["Authorization"] = std::string ("Basic ") + auth;

    // Send request

    const int RPC_REPLY_MAX_BYTES (256*1024*1024);
    using namespace std::chrono_literals;
    auto constexpr RPC_NOTIFY = 10min;

    HTTPClient::request (
        bSSL,
        io_service,
        strIp,
        iPort,
        std::bind (
            &RPCCallImp::onRequest,
            strMethod,
            jvParams,
            mapRequestHeaders,
            strPath, std::placeholders::_1, std::placeholders::_2, j),
        RPC_REPLY_MAX_BYTES,
        RPC_NOTIFY,
        std::bind (&RPCCallImp::onResponse, callbackFuncP,
                   std::placeholders::_1, std::placeholders::_2,
                   std::placeholders::_3, j, strMethod == "download"),
        j);
}

} // RPCCall

} //
