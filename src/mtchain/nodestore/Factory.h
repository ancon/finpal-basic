//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_FACTORY_H_INCLUDED
#define MTCHAIN_NODESTORE_FACTORY_H_INCLUDED

#include <mtchain/nodestore/Backend.h>
#include <mtchain/nodestore/Scheduler.h>
#include <mtchain/beast/utility/Journal.h>

namespace mtchain {
namespace NodeStore {

/** Base class for backend factories. */
class Factory
{
public:
    virtual
    ~Factory() = default;

    /** Retrieve the name of this factory. */
    virtual
    std::string
    getName() const = 0;

    /** Create an instance of this factory's backend.

        @param keyBytes The fixed number of bytes per key.
        @param keyValues A set of key/value configuration pairs.
        @param scheduler The scheduler to use for running tasks.
        @return A pointer to the Backend object.
    */
    virtual
    std::unique_ptr <Backend>
    createInstance (size_t keyBytes, Section const& parameters,
        Scheduler& scheduler, beast::Journal journal) = 0;
};

}
}

#endif
