//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CONDITIONS_ERROR_H
#define MTCHAIN_CONDITIONS_ERROR_H

#include <system_error>
#include <string>

namespace mtchain {
namespace cryptoconditions {

enum class error
{
    generic = 1,
    unsupported_type,
    unsupported_subtype,
    unknown_type,
    unknown_subtype,
    fingerprint_size,
    incorrect_encoding,
    trailing_garbage,
    buffer_empty,
    buffer_overfull,
    buffer_underfull,
    malformed_encoding,
    short_preamble,
    unexpected_tag,
    long_tag,
    large_size,
    preimage_too_long
};

std::error_code
make_error_code(error ev);

} // cryptoconditions
} //

namespace std
{

template<>
struct is_error_code_enum<mtchain::cryptoconditions::error>
{
    static bool const value = true;
};

} // std

#endif
