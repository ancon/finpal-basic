//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_TYPES_H_INCLUDED
#define MTCHAIN_NODESTORE_TYPES_H_INCLUDED

#include <mtchain/nodestore/NodeObject.h>
#include <mtchain/basics/BasicConfig.h>
#include <vector>

namespace mtchain {
namespace NodeStore {

enum
{
    // This is only used to pre-allocate the array for
    // batch objects and does not affect the amount written.
    //
    batchWritePreallocationSize = 128
};

/** Return codes from Backend operations. */
enum Status
{
    ok,
    notFound,
    dataCorrupt,
    unknown,

    customCode = 100
};

/** A batch of NodeObjects to write at once. */
using Batch = std::vector <std::shared_ptr<NodeObject>>;
}
}

#endif
