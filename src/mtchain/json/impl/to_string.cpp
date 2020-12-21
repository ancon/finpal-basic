//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/json_writer.h>
#include <mtchain/json/to_string.h>

namespace Json
{

std::string to_string (Value const& value)
{
    return FastWriter ().write (value);
}

std::string pretty (Value const& value)
{
    return StyledWriter().write (value);
}

} // namespace Json
