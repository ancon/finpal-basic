//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/nodestore/NodeObject.h>
#include <memory>

namespace mtchain {

//------------------------------------------------------------------------------

NodeObject::NodeObject (
    NodeObjectType type,
    Blob&& data,
    uint256 const& hash,
    PrivateAccess)
    : mType (type)
    , mHash (hash)
{
    mData = std::move (data);
}

std::shared_ptr<NodeObject>
NodeObject::createObject (
    NodeObjectType type,
    Blob&& data,
    uint256 const& hash)
{
    return std::make_shared <NodeObject> (
        type, std::move (data), hash, PrivateAccess ());
}

NodeObjectType
NodeObject::getType () const
{
    return mType;
}

uint256 const&
NodeObject::getHash () const
{
    return mHash;
}

Blob const&
NodeObject::getData () const
{
    return mData;
}

}
