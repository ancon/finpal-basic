//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_DETAIL_WORKPLAIN_H_INCLUDED
#define MTCHAIN_APP_MISC_DETAIL_WORKPLAIN_H_INCLUDED

#include <mtchain/app/misc/detail/WorkBase.h>

namespace mtchain {

namespace detail {

// Work over TCP/IP
class WorkPlain : public WorkBase<WorkPlain>
    , public std::enable_shared_from_this<WorkPlain>
{
    friend class WorkBase<WorkPlain>;

public:
    WorkPlain(
        std::string const& host,
        std::string const& path, std::string const& port,
        boost::asio::io_service& ios, callback_type cb);
    ~WorkPlain() = default;

private:
    void
    onConnect(error_code const& ec);

    socket_type&
    stream()
    {
        return socket_;
    }
};

//------------------------------------------------------------------------------

WorkPlain::WorkPlain(
    std::string const& host,
    std::string const& path, std::string const& port,
    boost::asio::io_service& ios, callback_type cb)
    : WorkBase (host, path, port, ios, cb)
{
}

void
WorkPlain::onConnect(error_code const& ec)
{
    if (ec)
        return fail(ec);

    onStart ();
}

} // detail

} //

#endif
