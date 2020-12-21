//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_DETAIL_WORK_H_INCLUDED
#define MTCHAIN_APP_MISC_DETAIL_WORK_H_INCLUDED

#include <beast/http/message.hpp>
#include <beast/http/string_body.hpp>

namespace mtchain {

namespace detail {

using response_type =
    beast::http::response<beast::http::string_body>;

class Work
{
public:
    virtual ~Work() = default;

    virtual void run() = 0;

    virtual void cancel() = 0;
};

} // detail

} //

#endif
