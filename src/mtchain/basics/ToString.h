//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_TOSTRING_H_INCLUDED
#define MTCHAIN_BASICS_TOSTRING_H_INCLUDED

#include <string>
#include <type_traits>

namespace mtchain {

/** to_string() generalizes std::to_string to handle bools, chars, and strings.

    It's also possible to provide implementation of to_string for a class
    which needs a string implementation.
 */

template <class T>
typename std::enable_if<std::is_arithmetic<T>::value,
                        std::string>::type
to_string(T t)
{
    return std::to_string(t);
}

inline std::string to_string(bool b)
{
    return b ? "true" : "false";
}

inline std::string to_string(char c)
{
    return std::string(1, c);
}

inline std::string to_string(std::string s)
{
    return s;
}

inline std::string to_string(char const* s)
{
    return s;
}

} //

#endif
