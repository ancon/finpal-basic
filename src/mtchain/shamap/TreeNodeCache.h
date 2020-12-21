//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SHAMAP_TREENODECACHE_H_INCLUDED
#define MTCHAIN_SHAMAP_TREENODECACHE_H_INCLUDED

#include <mtchain/shamap/TreeNodeCache.h>
#include <mtchain/shamap/SHAMapTreeNode.h>

namespace mtchain {

class SHAMapAbstractNode;

using TreeNodeCache = TaggedCache <uint256, SHAMapAbstractNode>;

} //

#endif
