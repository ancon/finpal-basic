//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_WSINFOSUB_H
#define MTCHAIN_RPC_WSINFOSUB_H

#include <mtchain/server/WSSession.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/beast/net/IPAddressConversion.h>
#include <mtchain/json/Output.h>
#include <mtchain/json/to_string.h>
#include <mtchain/rpc/Role.h>
#include <memory>
#include <string>

namespace mtchain {

class WSInfoSub : public InfoSub
{
    std::weak_ptr<WSSession> ws_;
    std::string user_;
    std::string fwdfor_;

public:
    WSInfoSub(Source& source, std::shared_ptr<WSSession> const& ws)
        : InfoSub(source)
        , ws_(ws)
    {
        auto const& h = ws->request().fields;
        auto it = h.find("X-User");
        if (it != h.end() &&
            isIdentified(
                ws->port(), beast::IPAddressConversion::from_asio(
                    ws->remote_endpoint()).address(), it->second))
        {
            user_ = it->second;
            it = h.find("X-Forwarded-For");
            if (it != h.end())
                fwdfor_ = it->second;
        }
    }

    std::string
    user() const
    {
        return user_;
    }

    std::string
    forwarded_for() const
    {
        return fwdfor_;
    }

    void
    send(Json::Value const& jv, bool)
    {
        auto sp = ws_.lock();
        if(! sp)
            return;
        beast::streambuf sb;
        stream(jv,
            [&](void const* data, std::size_t n)
            {
                sb.commit(boost::asio::buffer_copy(
                    sb.prepare(n), boost::asio::buffer(data, n)));
            });
        auto m = std::make_shared<
            StreambufWSMsg<decltype(sb)>>(
                std::move(sb));
        sp->send(m);
    }
};

} //

#endif
