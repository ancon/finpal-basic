//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_GOSSIP_H_INCLUDED
#define MTCHAIN_RESOURCE_GOSSIP_H_INCLUDED

#include <mtchain/beast/net/IPEndpoint.h>

namespace mtchain {
namespace Resource {

/** Data format for exchanging consumption information across peers. */
struct Gossip
{
    /** Describes a single consumer. */
    struct Item
    {
        int balance;
        beast::IP::Endpoint address;
    };

    std::vector <Item> items;
};

}
}

#endif
