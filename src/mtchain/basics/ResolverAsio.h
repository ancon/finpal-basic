//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_RESOLVERASIO_H_INCLUDED
#define MTCHAIN_BASICS_RESOLVERASIO_H_INCLUDED

#include <mtchain/basics/Resolver.h>
#include <mtchain/beast/utility/Journal.h>
#include <boost/asio/io_service.hpp>

namespace mtchain {

class ResolverAsio : public Resolver
{
public:
    static
    std::unique_ptr<ResolverAsio> New (
        boost::asio::io_service&, beast::Journal);
};

}

#endif
