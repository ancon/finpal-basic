//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/nodestore/impl/DatabaseRotatingImp.h>

namespace mtchain {
namespace NodeStore {

// Make sure to call it already locked!
std::shared_ptr <Backend> DatabaseRotatingImp::rotateBackends (
        std::shared_ptr <Backend> const& newBackend)
{
    std::shared_ptr <Backend> oldBackend = archiveBackend_;
    archiveBackend_ = writableBackend_;
    writableBackend_ = newBackend;

    return oldBackend;
}

std::shared_ptr<NodeObject> DatabaseRotatingImp::fetchFrom (uint256 const& hash)
{
    Backends b = getBackends();
    std::shared_ptr<NodeObject> object = fetchInternal (*b.writableBackend, hash);
    if (!object)
    {
        object = fetchInternal (*b.archiveBackend, hash);
        if (object)
        {
            getWritableBackend()->store (object);
            m_negCache.erase (hash);
        }
    }

    return object;
}
}

}
