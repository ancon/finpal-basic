//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_ACCOUNTSTATESF_H_INCLUDED
#define MTCHAIN_APP_LEDGER_ACCOUNTSTATESF_H_INCLUDED

#include <mtchain/app/ledger/AbstractFetchPackContainer.h>
#include <mtchain/shamap/SHAMapSyncFilter.h>
#include <mtchain/shamap/Family.h>

namespace mtchain {

// This class is only needed on add functions
// sync filter for account state nodes during ledger sync
class AccountStateSF
    : public SHAMapSyncFilter
{
private:
    Family& f_;
    AbstractFetchPackContainer& fp_;

public:
    AccountStateSF(Family&, AbstractFetchPackContainer&);

    // Note that the nodeData is overwritten by this call
    void gotNode (bool fromFilter,
                  SHAMapHash const& nodeHash,
                  Blob&& nodeData,
                  SHAMapTreeNode::TNType) const override;

    boost::optional<Blob>
    getNode(SHAMapHash const& nodeHash) const override;
};

} //

#endif
