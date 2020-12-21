//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_DISPOSITION_H_INCLUDED
#define MTCHAIN_RESOURCE_DISPOSITION_H_INCLUDED

namespace mtchain {
namespace Resource {

/** The disposition of a consumer after applying a load charge. */
enum Disposition
{
    /** No action required. */
    ok

    /** Consumer should be warned that consumption is high. */
    ,warn

    /** Consumer should be disconnected for excess consumption. */
    ,drop
};

}
}

#endif
