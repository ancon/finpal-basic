//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_BLOB_H_INCLUDED
#define MTCHAIN_BASICS_BLOB_H_INCLUDED

#include <vector>

namespace mtchain {

/** Storage for linear binary data.
    Blocks of binary data appear often in various idioms and structures.
*/
using Blob = std::vector <unsigned char>;

}

#endif
