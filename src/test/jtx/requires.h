//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_REQUIRES_H_INCLUDED
#define MTCHAIN_TEST_JTX_REQUIRES_H_INCLUDED

#include <functional>
#include <vector>

namespace mtchain {
namespace test {
namespace jtx {

class Env;

using require_t = std::function<void(Env&)>;
using requires_t = std::vector<require_t>;

} // jtx
} // test
} //

#endif
