//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/beast/core/Config.h>

#ifdef _MSC_VER
#include <cstddef>
namespace snappy {
using ssize_t = std::ptrdiff_t;
}
#endif

#if BEAST_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#include <snappy/snappy/snappy.cc>
#include <snappy/snappy/snappy-sinksource.cc>
#include <snappy/snappy/snappy-stubs-internal.cc>

#if BEAST_CLANG
#pragma clang diagnostic pop
#endif
