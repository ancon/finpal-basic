//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/AccountStateSF.h>

namespace mtchain {

AccountStateSF::AccountStateSF(Family& f, AbstractFetchPackContainer& fp)
    : f_(f)
    , fp_(fp)
{
}

void AccountStateSF::gotNode (bool fromFilter,
                              SHAMapHash const& nodeHash,
                              Blob&& nodeData,
                              SHAMapTreeNode::TNType) const
{
    f_.db().store(hotACCOUNT_NODE, std::move(nodeData),
        nodeHash.as_uint256());
}

boost::optional<Blob>
AccountStateSF::getNode(SHAMapHash const& nodeHash) const
{
    return fp_.getFetchPack(nodeHash.as_uint256());
}

} //
