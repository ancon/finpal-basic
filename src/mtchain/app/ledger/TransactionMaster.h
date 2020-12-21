//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_TRANSACTIONMASTER_H_INCLUDED
#define MTCHAIN_APP_LEDGER_TRANSACTIONMASTER_H_INCLUDED

#include <mtchain/shamap/SHAMapItem.h>
#include <mtchain/shamap/SHAMapTreeNode.h>

namespace mtchain {

class Application;
class Transaction;
class STTx;

// Tracks all transactions in memory

class TransactionMaster
{
public:
    TransactionMaster (Application& app);
    TransactionMaster (TransactionMaster const&) = delete;
    TransactionMaster& operator= (TransactionMaster const&) = delete;

    std::shared_ptr<Transaction>
    fetch (uint256 const& , bool checkDisk);

    std::shared_ptr<STTx const>
    fetch (std::shared_ptr<SHAMapItem> const& item,
        SHAMapTreeNode::TNType type, bool checkDisk,
            std::uint32_t uCommitLedger);

    // return value: true = we had the transaction already
    bool inLedger (uint256 const& hash, std::uint32_t ledger);

    void canonicalize (std::shared_ptr<Transaction>* pTransaction);

    void sweep (void);

    TaggedCache <uint256, Transaction>&
    getCache();

private:
    Application& mApp;
    TaggedCache <uint256, Transaction> mCache;
};

} //

#endif
