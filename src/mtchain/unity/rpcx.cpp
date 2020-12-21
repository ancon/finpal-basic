//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>

// This has to be included early to prevent an obscure MSVC compile error
#include <boost/asio/deadline_timer.hpp>

#include <mtchain/protocol/JsonFields.h>

#include <mtchain/rpc/RPCHandler.h>

#include <mtchain/rpc/impl/RPCHandler.cpp>
#include <mtchain/rpc/impl/Status.cpp>

#include <mtchain/rpc/handlers/Handlers.h>
#include <mtchain/rpc/handlers/AccountCurrenciesHandler.cpp>
#include <mtchain/rpc/handlers/AccountInfo.cpp>
#include <mtchain/rpc/handlers/AccountLines.cpp>
#include <mtchain/rpc/handlers/AccountChannels.cpp>
#include <mtchain/rpc/handlers/AccountObjects.cpp>
#include <mtchain/rpc/handlers/AccountOffers.cpp>
#include <mtchain/rpc/handlers/AccountTx.cpp>
#include <mtchain/rpc/handlers/AccountTxOld.cpp>
#include <mtchain/rpc/handlers/AccountTxSwitch.cpp>
#include <mtchain/rpc/handlers/BlackList.cpp>
#include <mtchain/rpc/handlers/BookOffers.cpp>
#include <mtchain/rpc/handlers/CanDelete.cpp>
#include <mtchain/rpc/handlers/Connect.cpp>
#include <mtchain/rpc/handlers/ConsensusInfo.cpp>
#include <mtchain/rpc/handlers/Feature1.cpp>
#include <mtchain/rpc/handlers/Fee1.cpp>
#include <mtchain/rpc/handlers/FetchInfo.cpp>
#include <mtchain/rpc/handlers/GatewayBalances.cpp>
#include <mtchain/rpc/handlers/GetCounts.cpp>
#include <mtchain/rpc/handlers/LedgerHandler.cpp>
#include <mtchain/rpc/handlers/LedgerAccept.cpp>
#include <mtchain/rpc/handlers/LedgerCleanerHandler.cpp>
#include <mtchain/rpc/handlers/LedgerClosed.cpp>
#include <mtchain/rpc/handlers/LedgerCurrent.cpp>
#include <mtchain/rpc/handlers/LedgerData.cpp>
#include <mtchain/rpc/handlers/LedgerEntry.cpp>
#include <mtchain/rpc/handlers/LedgerHeader.cpp>
#include <mtchain/rpc/handlers/LedgerRequest.cpp>
#include <mtchain/rpc/handlers/LogLevel.cpp>
#include <mtchain/rpc/handlers/LogRotate.cpp>
#include <mtchain/rpc/handlers/NoMTChainCheck.cpp>
#include <mtchain/rpc/handlers/OwnerInfo.cpp>
#include <mtchain/rpc/handlers/PathFind.cpp>
#include <mtchain/rpc/handlers/PayChanClaim.cpp>
#include <mtchain/rpc/handlers/Peers.cpp>
#include <mtchain/rpc/handlers/Ping.cpp>
#include <mtchain/rpc/handlers/Print.cpp>
#include <mtchain/rpc/handlers/Random.cpp>
#include <mtchain/rpc/handlers/MTChainPathFind.cpp>
#include <mtchain/rpc/handlers/ServerInfo.cpp>
#include <mtchain/rpc/handlers/ServerState.cpp>
#include <mtchain/rpc/handlers/SignFor.cpp>
#include <mtchain/rpc/handlers/SignHandler.cpp>
#include <mtchain/rpc/handlers/Stop.cpp>
#include <mtchain/rpc/handlers/Submit.cpp>
#include <mtchain/rpc/handlers/SubmitMultiSigned.cpp>
#include <mtchain/rpc/handlers/Subscribe.cpp>
#include <mtchain/rpc/handlers/TransactionEntry.cpp>
#include <mtchain/rpc/handlers/Tx.cpp>
#include <mtchain/rpc/handlers/TxHistory.cpp>
#include <mtchain/rpc/handlers/UnlList.cpp>
#include <mtchain/rpc/handlers/Unsubscribe.cpp>
#include <mtchain/rpc/handlers/ValidationCreate.cpp>
#include <mtchain/rpc/handlers/ValidationSeed.cpp>
#include <mtchain/rpc/handlers/WalletPropose.cpp>
#include <mtchain/rpc/handlers/WalletSeed.cpp>

#ifndef _MSC_VER
#include <mtchain/rpc/handlers/SmartContract.cpp>
#endif

#include <mtchain/rpc/impl/Handler.cpp>
#include <mtchain/rpc/impl/LegacyPathFind.cpp>
#include <mtchain/rpc/impl/Role.cpp>
#include <mtchain/rpc/impl/RPCHelpers.cpp>
#include <mtchain/rpc/impl/ServerHandlerImp.cpp>
#include <mtchain/rpc/impl/TransactionSign.cpp>
#include <mtchain/rpc/handlers/IpfsFeeInfo.cpp>
#include <mtchain/rpc/handlers/NFAssetInfo.cpp>
#include <mtchain/rpc/handlers/NFTokenInfo.cpp>

