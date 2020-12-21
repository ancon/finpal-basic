//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_HANDOFF_H_INCLUDED
#define MTCHAIN_SERVER_HANDOFF_H_INCLUDED

#include <mtchain/server/Writer.h>
#include <beast/http/message.hpp>
#include <beast/http/streambuf_body.hpp>
#include <memory>

namespace mtchain {

using http_request_type =
    beast::http::request<beast::http::streambuf_body>;

using http_response_type =
    beast::http::response<beast::http::streambuf_body>;

/** Used to indicate the result of a server connection handoff. */
struct Handoff
{
    // When `true`, the Session will close the socket. The
    // Handler may optionally take socket ownership using std::move
    bool moved = false;

    // If response is set, this determines the keep alive
    bool keep_alive = false;

    // When set, this will be sent back
    std::shared_ptr<Writer> response;

    bool handled() const
    {
        return moved || response;
    }
};

} //

#endif
