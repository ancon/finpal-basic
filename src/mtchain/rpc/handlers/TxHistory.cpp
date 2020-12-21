//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/Transaction.h>
#include <mtchain/core/DatabaseCon.h>
#include <mtchain/core/SociDB.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/Role.h>
#include <boost/format.hpp>

namespace mtchain {

// {
//   start: <index>
// }
Json::Value doTxHistory (RPC::Context& context)
{
    context.loadType = Resource::feeMediumBurdenRPC;

    if (!context.params.isMember (jss::start))
        return rpcError (rpcINVALID_PARAMS);

    unsigned int startIndex = context.params[jss::start].asUInt ();

    if ((startIndex > 10000) &&  (! isUnlimited (context.role)))
        return rpcError (rpcNO_PERMISSION);

    Json::Value obj;
    Json::Value txs;

    obj[jss::index] = startIndex;

    std::string sql =
        boost::str (boost::format (
            "SELECT LedgerSeq, Status, RawTxn "
            "FROM Transactions ORDER BY LedgerSeq desc LIMIT %u,20;")
                    % startIndex);

    {
        auto db = context.app.getTxnDB ().checkoutDb ();

        boost::optional<std::uint64_t> ledgerSeq;
        boost::optional<std::string> status;
        soci::blob sociRawTxnBlob (*db);
        soci::indicator rti;
        Blob rawTxn;

        soci::statement st = (db->prepare << sql,
                              soci::into (ledgerSeq),
                              soci::into (status),
                              soci::into (sociRawTxnBlob, rti));

        st.execute ();
        while (st.fetch ())
        {
            if (soci::i_ok == rti)
                convert(sociRawTxnBlob, rawTxn);
            else
                rawTxn.clear ();

            if (auto trans = Transaction::transactionFromSQL (
                    ledgerSeq, status, rawTxn, context.app))
                txs.append (trans->getJson (0));
        }
    }

    obj[jss::txs] = txs;

    return obj;
}

} //
