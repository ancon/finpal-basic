//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_DATABASEROTATING_H_INCLUDED
#define MTCHAIN_NODESTORE_DATABASEROTATING_H_INCLUDED

#include <mtchain/nodestore/Database.h>

namespace mtchain {
namespace NodeStore {

/* This class has two key-value store Backend objects for persisting SHAMap
 * records. This facilitates online deletion of data. New backends are
 * rotated in. Old ones are rotated out and deleted.
 */

class DatabaseRotating
{
public:
    virtual ~DatabaseRotating() = default;

    virtual TaggedCache <uint256, NodeObject>& getPositiveCache() = 0;

    virtual std::mutex& peekMutex() const = 0;

    virtual std::shared_ptr <Backend> const& getWritableBackend() const = 0;

    virtual std::shared_ptr <Backend> const& getArchiveBackend () const = 0;

    virtual std::shared_ptr <Backend> rotateBackends (
            std::shared_ptr <Backend> const& newBackend) = 0;

    /** Ensure that node is in writableBackend */
    virtual std::shared_ptr<NodeObject> fetchNode (uint256 const& hash) = 0;
};

}
}

#endif
