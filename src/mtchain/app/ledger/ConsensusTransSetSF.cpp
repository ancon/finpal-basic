//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/ConsensusTransSetSF.h>
#include <mtchain/app/ledger/TransactionMaster.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/app/misc/Transaction.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/nodestore/Database.h>
#include <mtchain/protocol/HashPrefix.h>

namespace mtchain {

ConsensusTransSetSF::ConsensusTransSetSF (Application& app, NodeCache& nodeCache)
    : app_ (app)
    , m_nodeCache (nodeCache)
    , j_ (app.journal ("TransactionAcquire"))
{
}

void ConsensusTransSetSF::gotNode (
    bool fromFilter, SHAMapHash const& nodeHash,
    Blob&& nodeData, SHAMapTreeNode::TNType type) const
{
    if (fromFilter)
        return;

    m_nodeCache.insert (nodeHash, nodeData);

    if ((type == SHAMapTreeNode::tnTRANSACTION_NM) && (nodeData.size () > 16))
    {
        // this is a transaction, and we didn't have it
        JLOG (j_.debug())
                << "Node on our acquiring TX set is TXN we may not have";

        try
        {
            // skip prefix
            Serializer s (nodeData.data() + 4, nodeData.size() - 4);
            SerialIter sit (s.slice());
            auto stx = std::make_shared<STTx const> (std::ref (sit));
            assert (stx->getTransactionID () == nodeHash.as_uint256());
            auto const pap = &app_;
            app_.getJobQueue ().addJob (
                jtTRANSACTION, "TXS->TXN",
                [pap, stx] (Job&) {
                    pap->getOPs().submitTransaction(stx);
                });
        }
        catch (std::exception const&)
        {
            JLOG (j_.warn())
                    << "Fetched invalid transaction in proposed set";
        }
    }
}

boost::optional<Blob>
ConsensusTransSetSF::getNode (SHAMapHash const& nodeHash) const
{
    Blob nodeData;
    if (m_nodeCache.retrieve (nodeHash, nodeData))
        return nodeData;

    auto txn = app_.getMasterTransaction().fetch(nodeHash.as_uint256(), false);

    if (txn)
    {
        // this is a transaction, and we have it
        JLOG (j_.trace())
                << "Node in our acquiring TX set is TXN we have";
        Serializer s;
        s.add32 (HashPrefix::transactionID);
        txn->getSTransaction ()->add (s);
        assert(sha512Half(s.slice()) == nodeHash.as_uint256());
        nodeData = s.peekData ();
        return nodeData;
    }

    return boost::none;
}

} //
