//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_ACCEPTEDLEDGERTX_H_INCLUDED
#define MTCHAIN_APP_LEDGER_ACCEPTEDLEDGERTX_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/protocol/AccountID.h>
#include <boost/container/flat_set.hpp>

namespace mtchain {

class Logs;

/**
    A transaction that is in a closed ledger.

    Description

    An accepted ledger transaction contains additional information that the
    server needs to tell clients about the transaction. For example,
        - The transaction in JSON form
        - Which accounts are affected
          * This is used by InfoSub to report to clients
        - Cached stuff

    @code
    @endcode

    @see {uri}

    @ingroup mtchain_ledger
*/
class AcceptedLedgerTx
{
public:
    using pointer = std::shared_ptr <AcceptedLedgerTx>;
    using ref = const pointer&;

public:
    AcceptedLedgerTx (
        std::shared_ptr<ReadView const> const& ledger,
        std::shared_ptr<STTx const> const&,
        std::shared_ptr<STObject const> const&,
        AccountIDCache const&,
        Logs&);
    AcceptedLedgerTx (
        std::shared_ptr<ReadView const> const&,
        std::shared_ptr<STTx const> const&,
        TER,
        AccountIDCache const&,
        Logs&);

    std::shared_ptr <STTx const> const& getTxn () const
    {
        return mTxn;
    }
    std::shared_ptr <TxMeta> const& getMeta () const
    {
        return mMeta;
    }

    boost::container::flat_set<AccountID> const&
    getAffected() const
    {
        return mAffected;
    }

    TxID getTransactionID () const
    {
        return mTxn->getTransactionID ();
    }
    TxType getTxnType () const
    {
        return mTxn->getTxnType ();
    }
    TER getResult () const
    {
        return mResult;
    }
    std::uint32_t getTxnSeq () const
    {
        return mMeta->getIndex ();
    }

    bool isApplied () const
    {
        return bool(mMeta);
    }
    int getIndex () const
    {
        return mMeta ? mMeta->getIndex () : 0;
    }
    std::string getEscMeta () const;
    Json::Value getJson () const
    {
        return mJson;
    }

private:
    std::shared_ptr<ReadView const> mLedger;
    std::shared_ptr<STTx const> mTxn;
    std::shared_ptr<TxMeta> mMeta;
    TER                             mResult;
    boost::container::flat_set<AccountID> mAffected;
    Blob        mRawMeta;
    Json::Value                     mJson;
    AccountIDCache const& accountCache_;
    Logs& logs_;

    void buildJson ();
};

} //

#endif
