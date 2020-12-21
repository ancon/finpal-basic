//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_TUNING_H_INCLUDED
#define MTCHAIN_NODESTORE_TUNING_H_INCLUDED

namespace mtchain {
namespace NodeStore {

enum
{
    // Target cache size of the TaggedCache used to hold nodes
    cacheTargetSize     = 16384

    // Expiration time for cached nodes
    ,cacheTargetSeconds = 300

    // Fraction of the cache one query source can take
    ,asyncDivider = 8
};

}
}

#endif
