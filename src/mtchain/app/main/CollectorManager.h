//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MAIN_COLLECTORMANAGER_H_INCLUDED
#define MTCHAIN_APP_MAIN_COLLECTORMANAGER_H_INCLUDED

#include <mtchain/basics/BasicConfig.h>
#include <mtchain/beast/insight/Insight.h>

namespace mtchain {

/** Provides the beast::insight::Collector service. */
class CollectorManager
{
public:
    static std::unique_ptr<CollectorManager> New (
        Section const& params, beast::Journal journal);

    virtual ~CollectorManager () = 0;
    virtual beast::insight::Collector::ptr const& collector () = 0;
    virtual beast::insight::Group::ptr const& group (
        std::string const& name) = 0;
};

}

#endif
