//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_SERVERHANDLER_H_INCLUDED
#define MTCHAIN_RPC_SERVERHANDLER_H_INCLUDED

#include <mtchain/basics/BasicConfig.h>
#include <mtchain/core/Config.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/core/Stoppable.h>
#include <mtchain/server/Port.h>
#include <mtchain/resource/ResourceManager.h>
#include <mtchain/rpc/impl/ServerHandlerImp.h>
#include <mtchain/beast/utility/Journal.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <memory>
#include <vector>

namespace mtchain {

using ServerHandler = ServerHandlerImp;

ServerHandler::Setup
setup_ServerHandler (
    Config const& c,
    std::ostream&& log);

std::unique_ptr <ServerHandler>
make_ServerHandler (Application& app, Stoppable& parent, boost::asio::io_service&,
    JobQueue&, NetworkOPs&, Resource::Manager&,
        CollectorManager& cm);

} //

#endif
