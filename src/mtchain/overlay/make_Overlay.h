//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_OVERLAY_MAKE_OVERLAY_H_INCLUDED
#define MTCHAIN_OVERLAY_MAKE_OVERLAY_H_INCLUDED

#include <mtchain/rpc/ServerHandler.h>
#include <mtchain/overlay/Overlay.h>
#include <mtchain/resource/ResourceManager.h>
#include <mtchain/basics/Resolver.h>
#include <mtchain/core/Stoppable.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl/context.hpp>

namespace mtchain {

Overlay::Setup
setup_Overlay (BasicConfig const& config);

/** Creates the implementation of Overlay. */
std::unique_ptr <Overlay>
make_Overlay (
    Application& app,
    Overlay::Setup const& setup,
    Stoppable& parent,
    ServerHandler& serverHandler,
    Resource::Manager& resourceManager,
    Resolver& resolver,
    boost::asio::io_service& io_service,
    BasicConfig const& config);

} //

#endif
