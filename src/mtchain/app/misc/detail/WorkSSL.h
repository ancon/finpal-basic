//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_DETAIL_WORKSSL_H_INCLUDED
#define MTCHAIN_APP_MISC_DETAIL_WORKSSL_H_INCLUDED

#include <mtchain/app/misc/detail/WorkBase.h>
#include <mtchain/basics/contract.h>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

namespace mtchain {

namespace detail {

class SSLContext : public boost::asio::ssl::context
{
public:
    SSLContext()
    : boost::asio::ssl::context(boost::asio::ssl::context::sslv23)
    {
        boost::system::error_code ec;
        set_default_verify_paths (ec);

        if (ec)
        {
            Throw<std::runtime_error> (
                boost::str (boost::format (
                    "Failed to set_default_verify_paths: %s") %
                    ec.message ()));
        }
    }
};

// Work over SSL
class WorkSSL : public WorkBase<WorkSSL>
    , public std::enable_shared_from_this<WorkSSL>
{
    friend class WorkBase<WorkSSL>;

private:
    using stream_type = boost::asio::ssl::stream<socket_type&>;

    SSLContext context_;
    stream_type stream_;

public:
    WorkSSL(
        std::string const& host,
        std::string const& path, std::string const& port,
        boost::asio::io_service& ios, callback_type cb);
    ~WorkSSL() = default;

private:
    stream_type&
    stream()
    {
        return stream_;
    }

    void
    onConnect(error_code const& ec);

    void
    onHandshake(error_code const& ec);

    static bool
    rfc2818_verify (
        std::string const& domain,
        bool preverified,
        boost::asio::ssl::verify_context& ctx)
    {
        return
            boost::asio::ssl::rfc2818_verification (domain) (preverified, ctx);
    }
};

//------------------------------------------------------------------------------

WorkSSL::WorkSSL(
    std::string const& host,
    std::string const& path, std::string const& port,
    boost::asio::io_service& ios, callback_type cb)
    : WorkBase (host, path, port, ios, cb)
    , context_()
    , stream_ (socket_, context_)
{
    stream_.set_verify_mode (boost::asio::ssl::verify_peer);
    stream_.set_verify_callback (
        std::bind (
            &WorkSSL::rfc2818_verify, host_,
            std::placeholders::_1, std::placeholders::_2));
}

void
WorkSSL::onConnect(error_code const& ec)
{
    if (ec)
        return fail(ec);

    stream_.async_handshake(
        boost::asio::ssl::stream_base::client,
        strand_.wrap (boost::bind(&WorkSSL::onHandshake, shared_from_this(),
            boost::asio::placeholders::error)));
}

void
WorkSSL::onHandshake(error_code const& ec)
{
    if (ec)
        return fail(ec);

    onStart ();
}

} // detail

} //

#endif
