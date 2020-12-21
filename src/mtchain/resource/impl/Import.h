//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_IMPORT_H_INCLUDED
#define MTCHAIN_RESOURCE_IMPORT_H_INCLUDED

#include <mtchain/resource/Consumer.h>

namespace mtchain {
namespace Resource {

/** A set of imported consumer data from a gossip origin. */
struct Import
{
    struct Item
    {
        int balance;
        Consumer consumer;
    };

    // Dummy argument required for zero-copy construction
    Import (int = 0)
        : whenExpires (0)
    {
    }

    // When the imported data expires
    clock_type::rep whenExpires;

    // List of remote entries
    std::vector <Item> items;
};

}
}

#endif
