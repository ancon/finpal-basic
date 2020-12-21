//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SHAMAP_SHAMAPSYNCFILTER_H_INCLUDED
#define MTCHAIN_SHAMAP_SHAMAPSYNCFILTER_H_INCLUDED

#include <mtchain/shamap/SHAMapNodeID.h>
#include <mtchain/shamap/SHAMapTreeNode.h>

/** Callback for filtering SHAMap during sync. */
namespace mtchain {

class SHAMapSyncFilter
{
public:
    virtual ~SHAMapSyncFilter () = default;
    SHAMapSyncFilter() = default;
    SHAMapSyncFilter(SHAMapSyncFilter const&) = delete;
    SHAMapSyncFilter& operator=(SHAMapSyncFilter const&) = delete;

    // Note that the nodeData is overwritten by this call
    virtual void gotNode (bool fromFilter,
                          SHAMapHash const& nodeHash,
                          Blob&& nodeData,
                          SHAMapTreeNode::TNType type) const = 0;

    virtual
    boost::optional<Blob>
    getNode(SHAMapHash const& nodeHash) const = 0;
};

}

#endif
