//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_ENCODEDBLOB_H_INCLUDED
#define MTCHAIN_NODESTORE_ENCODEDBLOB_H_INCLUDED

#include <mtchain/basics/Buffer.h>
#include <mtchain/nodestore/NodeObject.h>
#include <cstddef>

namespace mtchain {
namespace NodeStore {

/** Utility for producing flattened node objects.
    @note This defines the database format of a NodeObject!
*/
// VFALCO TODO Make allocator aware and use short_alloc
struct EncodedBlob
{
public:
    void prepare (std::shared_ptr<NodeObject> const& object);

    void const* getKey () const noexcept
    {
        return m_key;
    }

    std::size_t getSize () const noexcept
    {
        return m_data.size();
    }

    void const* getData () const noexcept
    {
        return reinterpret_cast<void const*>(m_data.data());
    }

private:
    void const* m_key;
    Buffer m_data;
};

}
}

#endif
