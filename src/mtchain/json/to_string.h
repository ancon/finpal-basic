//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_JSON_TO_STRING_H_INCLUDED
#define MTCHAIN_JSON_TO_STRING_H_INCLUDED

#include <string>
#include <ostream>

namespace Json {

class Value;

/** Writes a Json::Value to an std::string. */
std::string to_string (Value const&);

/** Writes a Json::Value to an std::string. */
std::string pretty (Value const&);

/** Output using the StyledStreamWriter. @see Json::operator>>(). */
std::ostream& operator<< (std::ostream&, const Value& root);

} // Json

#endif // JSON_TO_STRING_H_INCLUDED
