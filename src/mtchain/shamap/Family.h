//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SHAMAP_FAMILY_H_INCLUDED
#define MTCHAIN_SHAMAP_FAMILY_H_INCLUDED

#include <mtchain/basics/Log.h>
#include <mtchain/shamap/FullBelowCache.h>
#include <mtchain/shamap/TreeNodeCache.h>
#include <mtchain/nodestore/Database.h>
#include <mtchain/beast/utility/Journal.h>
#include <cstdint>

namespace mtchain {

class Family
{
public:
    virtual ~Family() = default;

    virtual
    beast::Journal const&
    journal() = 0;

    virtual
    FullBelowCache&
    fullbelow() = 0;

    virtual
    FullBelowCache const&
    fullbelow() const = 0;

    virtual
    TreeNodeCache&
    treecache() = 0;

    virtual
    TreeNodeCache const&
    treecache() const = 0;

    virtual
    NodeStore::Database&
    db() = 0;

    virtual
    NodeStore::Database const&
    db() const = 0;

    virtual
    void
    missing_node (std::uint32_t refNum) = 0;

    virtual
    void
    missing_node (uint256 const& refHash) = 0;
};

} //

#endif
