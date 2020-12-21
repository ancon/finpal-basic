//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_HANDLERS_HANDLERS_H_INCLUDED
#define MTCHAIN_RPC_HANDLERS_HANDLERS_H_INCLUDED

#include <mtchain/rpc/handlers/LedgerHandler.h>

namespace mtchain {

Json::Value doAccountCurrencies     (RPC::Context&);
Json::Value doAccountInfo           (RPC::Context&);
Json::Value doAccountLines          (RPC::Context&);
Json::Value doAccountChannels       (RPC::Context&);
Json::Value doAccountObjects        (RPC::Context&);
Json::Value doAccountOffers         (RPC::Context&);
Json::Value doAccountTx             (RPC::Context&);
Json::Value doAccountTxSwitch       (RPC::Context&);
Json::Value doAccountTxOld          (RPC::Context&);
Json::Value doBookOffers            (RPC::Context&);
Json::Value doBlackList             (RPC::Context&);
Json::Value doCanDelete             (RPC::Context&);
Json::Value doChannelAuthorize      (RPC::Context&);
Json::Value doChannelVerify         (RPC::Context&);
Json::Value doConnect               (RPC::Context&);
Json::Value doConsensusInfo         (RPC::Context&);
Json::Value doFeature               (RPC::Context&);
Json::Value doFee                   (RPC::Context&);
Json::Value doFetchInfo             (RPC::Context&);
Json::Value doGatewayBalances       (RPC::Context&);
Json::Value doGetCounts             (RPC::Context&);
Json::Value doIpfsFeeInfo           (RPC::Context&);
Json::Value doLedgerAccept          (RPC::Context&);
Json::Value doLedgerCleaner         (RPC::Context&);
Json::Value doLedgerClosed          (RPC::Context&);
Json::Value doLedgerCurrent         (RPC::Context&);
Json::Value doLedgerData            (RPC::Context&);
Json::Value doLedgerEntry           (RPC::Context&);
Json::Value doLedgerHeader          (RPC::Context&);
Json::Value doLedgerRequest         (RPC::Context&);
Json::Value doLogLevel              (RPC::Context&);
Json::Value doLogRotate             (RPC::Context&);
Json::Value doNoMTChainCheck         (RPC::Context&);
Json::Value doOwnerInfo             (RPC::Context&);
Json::Value doPathFind              (RPC::Context&);
Json::Value doPeers                 (RPC::Context&);
Json::Value doPing                  (RPC::Context&);
Json::Value doPrint                 (RPC::Context&);
Json::Value doRandom                (RPC::Context&);
Json::Value doMTChainPathFind        (RPC::Context&);
Json::Value doServerInfo            (RPC::Context&); // for humans
Json::Value doServerState           (RPC::Context&); // for machines
Json::Value doSessionClose          (RPC::Context&);
Json::Value doSessionOpen           (RPC::Context&);
Json::Value doSign                  (RPC::Context&);
Json::Value doSignFor               (RPC::Context&);
Json::Value doStop                  (RPC::Context&);
Json::Value doSubmit                (RPC::Context&);
Json::Value doSubmitMultiSigned     (RPC::Context&);
Json::Value doSubscribe             (RPC::Context&);
Json::Value doTransactionEntry      (RPC::Context&);
Json::Value doTx                    (RPC::Context&);
Json::Value doTxHistory             (RPC::Context&);
Json::Value doUnlList               (RPC::Context&);
Json::Value doUnsubscribe           (RPC::Context&);
Json::Value doValidationCreate      (RPC::Context&);
Json::Value doValidationSeed        (RPC::Context&);
Json::Value doWalletLock            (RPC::Context&);
Json::Value doWalletPropose         (RPC::Context&);
Json::Value doWalletSeed            (RPC::Context&);
Json::Value doWalletUnlock          (RPC::Context&);
Json::Value doWalletVerify          (RPC::Context&);
Json::Value doSmartContract         (RPC::Context&);// for smart constract
Json::Value doFileDownload          (RPC::Context&);
Json::Value doTokenInfo             (RPC::Context&);
Json::Value doAssetBalance          (RPC::Context&);
Json::Value doAssetInfo             (RPC::Context&);
Json::Value doIsAssetOperator       (RPC::Context&);
Json::Value doAssetAllTokenInfo     (RPC::Context&);
Json::Value doAccountAllAssetInfo   (RPC::Context&);
Json::Value doAccountAllTokenInfo   (RPC::Context&);
STAmount getGatewayAmount(std::string const& issuer, std::string const& currency);
} //

#endif
