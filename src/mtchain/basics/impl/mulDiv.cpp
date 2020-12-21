//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/mulDiv.h>
#include <mtchain/basics/contract.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <limits>
#include <utility>

namespace mtchain
{

std::pair<bool, std::uint64_t>
mulDiv(std::uint64_t value, std::uint64_t mul, std::uint64_t div)
{
    using namespace boost::multiprecision;

    uint128_t result;
    result = multiply(result, value, mul);

    result /= div;

    auto const limit = std::numeric_limits<std::uint64_t>::max();

    if (result > limit)
        return { false, limit };

    return { true, static_cast<std::uint64_t>(result) };
}

} //
