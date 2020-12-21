//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_VISITCALLBACK_H_INCLUDED
#define MTCHAIN_NODESTORE_VISITCALLBACK_H_INCLUDED

namespace mtchain {
namespace NodeStore {

/** Callback for iterating through objects.

    @see visitAll
*/
// VFALCO DEPRECATED Use std::function instead
struct VisitCallback
{
    virtual void visitObject (NodeObject::Ptr const& object) = 0;
};

}
}

#endif
