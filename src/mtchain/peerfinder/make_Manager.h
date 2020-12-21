//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_MAKE_MANAGER_H_INCLUDED
#define MTCHAIN_PEERFINDER_MAKE_MANAGER_H_INCLUDED

#include <mtchain/peerfinder/PeerfinderManager.h>
#include <boost/asio/io_service.hpp>
#include <memory>

namespace mtchain {
namespace PeerFinder {

/** Create a new Manager. */
std::unique_ptr<Manager>
make_Manager (Stoppable& parent, boost::asio::io_service& io_service,
        clock_type& clock, beast::Journal journal, BasicConfig const& config);

}
}

#endif
