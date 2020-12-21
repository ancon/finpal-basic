//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_SIMPLEWRITER_H_INCLUDED
#define MTCHAIN_SERVER_SIMPLEWRITER_H_INCLUDED

#include <mtchain/server/Writer.h>
#include <beast/core/streambuf.hpp>
#include <beast/core/write_dynabuf.hpp>
#include <beast/http/message.hpp>
#include <beast/http/write.hpp>
#include <utility>

namespace mtchain {

/// Deprecated: Writer that serializes a HTTP/1 message
class SimpleWriter : public Writer
{
    beast::streambuf sb_;

public:
    template<bool isRequest, class Body, class Headers>
    explicit
    SimpleWriter(beast::http::message<
        isRequest, Body, Headers> const& msg)
    {
        beast::write(sb_, msg);
    }

    bool
    complete() override
    {
        return sb_.size() == 0;
    }

    void
    consume (std::size_t bytes) override
    {
        sb_.consume(bytes);
    }

    bool
    prepare(std::size_t bytes,
        std::function<void(void)>) override
    {
        return true;
    }

    std::vector<boost::asio::const_buffer>
    data() override
    {
        auto const& buf = sb_.data();
        std::vector<boost::asio::const_buffer> result;
        result.reserve(std::distance(buf.begin(), buf.end()));
        for (auto const& b : buf)
            result.push_back(b);
        return result;
    }
};

} //

#endif
