//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/nodestore/impl/EncodedBlob.h>
#include <cstring>

namespace mtchain {
namespace NodeStore {

void
EncodedBlob::prepare (
    std::shared_ptr<NodeObject> const& object)
{
    m_key = object->getHash().begin ();

    auto ret = m_data.alloc(object->getData ().size () + 9);

    // the first 8 bytes are unused
    memset (ret, 0, 8);

    ret[8] = static_cast<std::uint8_t> (object->getType ());

    memcpy (ret + 9,
        object->getData ().data(),
        object->getData ().size());
}

}
}
