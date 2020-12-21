//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_KIND_H_INCLUDED
#define MTCHAIN_RESOURCE_KIND_H_INCLUDED

namespace mtchain {
namespace Resource {

/**
 * Kind of consumer.
 * kindInbound:   Inbound connection.
 * kindOutbound:  Outbound connection.
 * kindUnlimited: Inbound connection with no resource limits, but could be
 *                subjected to administrative restrictions, such as
 *                use of some RPC commands like "stop".
 */
enum Kind
{
     kindInbound
    ,kindOutbound
    ,kindUnlimited
};

}
}

#endif
