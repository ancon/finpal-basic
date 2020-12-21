//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NET_RPCSUB_H_INCLUDED
#define MTCHAIN_NET_RPCSUB_H_INCLUDED

#include <mtchain/core/JobQueue.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/core/Stoppable.h>
#include <boost/asio/io_service.hpp>

namespace mtchain {

/** Subscription object for JSON RPC. */
class RPCSub : public InfoSub
{
public:
    virtual void setUsername (std::string const& strUsername) = 0;
    virtual void setPassword (std::string const& strPassword) = 0;

protected:
    explicit RPCSub (InfoSub::Source& source);
};

// VFALCO Why is the io_service needed?
std::shared_ptr<RPCSub> make_RPCSub (
    InfoSub::Source& source, boost::asio::io_service& io_service,
    JobQueue& jobQueue, std::string const& strUrl,
    std::string const& strUsername, std::string const& strPassword,
    Logs& logs);

} //

#endif
