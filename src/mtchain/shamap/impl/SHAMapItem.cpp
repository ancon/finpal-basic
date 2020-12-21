//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/Serializer.h>
#include <mtchain/shamap/SHAMapItem.h>

namespace mtchain {

class SHAMap;

SHAMapItem::SHAMapItem (uint256 const& tag, Blob const& data)
    : tag_(tag)
    , data_(data)
{
}

SHAMapItem::SHAMapItem (uint256 const& tag, const Serializer& data)
    : tag_ (tag)
    , data_(data.peekData())
{
}

SHAMapItem::SHAMapItem (uint256 const& tag, Serializer&& data)
    : tag_ (tag)
    , data_(std::move(data.modData()))
{
}

} //
