//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/TransactionStateSF.h>

namespace mtchain {

TransactionStateSF::TransactionStateSF(Family& f,
    AbstractFetchPackContainer& fp)
        : f_(f)
        , fp_(fp)
{
}

void TransactionStateSF::gotNode (bool fromFilter,
                                  SHAMapHash const& nodeHash,
                                  Blob&& nodeData,
                                  SHAMapTreeNode::TNType type) const
{
    assert(type !=
        SHAMapTreeNode::tnTRANSACTION_NM);
    f_.db().store(hotTRANSACTION_NODE,
        std::move (nodeData), nodeHash.as_uint256());
}

boost::optional<Blob>
TransactionStateSF::getNode(SHAMapHash const& nodeHash) const
{
    return fp_.getFetchPack(nodeHash.as_uint256());
}

} //
