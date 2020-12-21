//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_CONSENSUSTRANSSETSF_H_INCLUDED
#define MTCHAIN_APP_LEDGER_CONSENSUSTRANSSETSF_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/shamap/SHAMapSyncFilter.h>
#include <mtchain/basics/TaggedCache.h>

namespace mtchain {

// Sync filters allow low-level SHAMapSync code to interact correctly with
// higher-level structures such as caches and transaction stores

// This class is needed on both add and check functions
// sync filter for transaction sets during consensus building
class ConsensusTransSetSF : public SHAMapSyncFilter
{
public:
    using NodeCache = TaggedCache <SHAMapHash, Blob>;

    ConsensusTransSetSF (Application& app, NodeCache& nodeCache);

    // Note that the nodeData is overwritten by this call
    void gotNode (bool fromFilter,
                  SHAMapHash const& nodeHash,
                  Blob&& nodeData,
                  SHAMapTreeNode::TNType) const override;

    boost::optional<Blob>
    getNode (SHAMapHash const& nodeHash) const override;

private:
    Application& app_;
    NodeCache& m_nodeCache;
    beast::Journal j_;
};

} //

#endif
