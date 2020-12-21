//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_DECODEDBLOB_H_INCLUDED
#define MTCHAIN_NODESTORE_DECODEDBLOB_H_INCLUDED

#include <mtchain/nodestore/NodeObject.h>

namespace mtchain {
namespace NodeStore {

/** Parsed key/value blob into NodeObject components.

    This will extract the information required to construct a NodeObject. It
    also does consistency checking and returns the result, so it is possible
    to determine if the data is corrupted without throwing an exception. Not
    all forms of corruption are detected so further analysis will be needed
    to eliminate false negatives.

    @note This defines the database format of a NodeObject!
*/
class DecodedBlob
{
public:
    /** Construct the decoded blob from raw data. */
    DecodedBlob (void const* key, void const* value, int valueBytes);

    /** Determine if the decoding was successful. */
    bool wasOk () const noexcept { return m_success; }

    /** Create a NodeObject from this data. */
    std::shared_ptr<NodeObject> createObject ();

private:
    bool m_success;

    void const* m_key;
    NodeObjectType m_objectType;
    unsigned char const* m_objectData;
    int m_dataBytes;
};

}
}

#endif
