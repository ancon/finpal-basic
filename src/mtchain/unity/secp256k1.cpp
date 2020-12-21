//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>

#define USE_NUM_NONE
#define USE_FIELD_10X26
#define USE_FIELD_INV_BUILTIN
#define USE_SCALAR_8X32
#define USE_SCALAR_INV_BUILTIN

#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning ( push )
#pragma warning ( disable: 4319 )
#endif
#include <secp256k1/src/secp256k1.c>
#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning ( pop )
#endif
