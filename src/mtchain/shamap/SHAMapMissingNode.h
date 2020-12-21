//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SHAMAP_SHAMAPMISSINGNODE_H_INCLUDED
#define MTCHAIN_SHAMAP_SHAMAPMISSINGNODE_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/shamap/SHAMapTreeNode.h>
#include <iosfwd>
#include <stdexcept>

namespace mtchain {

enum class SHAMapType
{
    TRANSACTION  = 1,    // A tree of transactions
    STATE        = 2,    // A tree of state nodes
    FREE         = 3,    // A tree not part of a ledger
};

class SHAMapMissingNode
    : public std::runtime_error
{
private:
    SHAMapType mType;
    SHAMapHash mNodeHash;
    uint256    mNodeID;
public:
    SHAMapMissingNode (SHAMapType t,
                       SHAMapHash const& nodeHash)
        : std::runtime_error ("SHAMapMissingNode")
        , mType (t)
        , mNodeHash (nodeHash)
    {
    }

    SHAMapMissingNode (SHAMapType t,
                       uint256 const& nodeID)
        : std::runtime_error ("SHAMapMissingNode")
        , mType (t)
        , mNodeID (nodeID)
    {
    }

    friend std::ostream& operator<< (std::ostream&, SHAMapMissingNode const&);
};

} //

#endif
