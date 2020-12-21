//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_PLAINWSPEER_H_INCLUDED
#define MTCHAIN_SERVER_PLAINWSPEER_H_INCLUDED

#include <mtchain/server/impl/BaseWSPeer.h>
#include <memory>

namespace mtchain {

template<class Handler>
class PlainWSPeer
    : public BaseWSPeer<Handler, PlainWSPeer<Handler>>
    , public std::enable_shared_from_this<PlainWSPeer<Handler>>
{
    friend class BasePeer<Handler, PlainWSPeer>;
    friend class BaseWSPeer<Handler, PlainWSPeer>;

    using clock_type = std::chrono::system_clock;
    using error_code = boost::system::error_code;
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using waitable_timer = boost::asio::basic_waitable_timer <clock_type>;
    using socket_type = boost::asio::ip::tcp::socket;

    beast::websocket::stream<socket_type> ws_;

public:
    template<class Body, class Headers>
    PlainWSPeer(
        Port const& port,
        Handler& handler,
        endpoint_type remote_address,
        beast::http::request<Body, Headers>&& request,
        socket_type&& socket,
        beast::Journal journal);
};

//------------------------------------------------------------------------------

template<class Handler>
template<class Body, class Headers>
PlainWSPeer<Handler>::
PlainWSPeer(
    Port const& port,
    Handler& handler,
    endpoint_type remote_address,
    beast::http::request<Body, Headers>&& request,
    socket_type&& socket,
    beast::Journal journal)
    : BaseWSPeer<Handler, PlainWSPeer>(port, handler, remote_address,
        std::move(request), socket.get_io_service(), journal)
    , ws_(std::move(socket))
{
}

} //

#endif
