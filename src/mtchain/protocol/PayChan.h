//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_PAYCHAN_H_INCLUDED
#define MTCHAIN_PROTOCOL_PAYCHAN_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/protocol/HashPrefix.h>
#include <mtchain/protocol/Serializer.h>
#include <mtchain/protocol/MAmount.h>

namespace mtchain {

inline
void
serializePayChanAuthorization (
    Serializer& msg,
    uint256 const& key,
    MAmount const& amt)
{
    msg.add32 (HashPrefix::paymentChannelClaim);
    msg.add256 (key);
    msg.add64 (amt.drops ());
}

} //

#endif
