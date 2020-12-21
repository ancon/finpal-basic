//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_JSON_BODY_H
#define MTCHAIN_RPC_JSON_BODY_H

#include <mtchain/json/json_value.h>
#include <beast/core/streambuf.hpp>
#include <beast/http/message.hpp>

namespace mtchain {

/// Body that holds JSON
struct json_body
{
    using value_type = Json::Value;

    class writer
    {
        beast::streambuf sb_;

    public:
        template<bool isRequest, class Fields>
        explicit
        writer(beast::http::message<
            isRequest, json_body, Fields> const& m) noexcept
        {
            stream(m.body,
                [&](void const* data, std::size_t n)
                {
                    sb_.commit(boost::asio::buffer_copy(
                        sb_.prepare(n), boost::asio::buffer(data, n)));
                });
        }

        void
        init(beast::error_code&) noexcept
        {
        }

        std::uint64_t
        content_length() const noexcept
        {
            return sb_.size();
        }

        template<class Writef>
        boost::tribool
        write(beast::http::resume_context&&,
            beast::error_code&, Writef&& wf) noexcept
        {
            wf(sb_.data());
            return true;
        }
    };
};

} //

#endif
