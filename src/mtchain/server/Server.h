//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_SERVER_H_INCLUDED
#define MTCHAIN_SERVER_SERVER_H_INCLUDED

#include <mtchain/server/Port.h>
#include <mtchain/server/impl/ServerImpl.h>
#include <mtchain/beast/utility/Journal.h>
#include <mtchain/beast/utility/PropertyStream.h>
#include <boost/asio/io_service.hpp>

namespace mtchain {

/** Create the HTTP server using the specified handler. */
template<class Handler>
std::unique_ptr<Server>
make_Server(Handler& handler,
    boost::asio::io_service& io_service, beast::Journal journal)
{
    return std::make_unique<ServerImpl<Handler>>(
        handler, io_service, journal);
}

} //

#endif
