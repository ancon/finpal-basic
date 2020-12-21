//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_KEY_H_INCLUDED
#define MTCHAIN_RESOURCE_KEY_H_INCLUDED

#include <mtchain/resource/impl/Kind.h>
#include <mtchain/beast/net/IPEndpoint.h>
#include <cassert>

namespace mtchain {
namespace Resource {

// The consumer key
struct Key
{
    Kind kind;
    beast::IP::Endpoint address;
    std::string name;

    Key () = delete;

    // Constructor for Inbound and Outbound (non-Unlimited) keys
    Key (Kind k, beast::IP::Endpoint const& addr)
        : kind(k)
        , address(addr)
    {
        assert(kind != kindUnlimited);
    }

    // Constructor for Unlimited keys
    Key (std::string const& n)
        : kind(kindUnlimited)
        , name(n)
    {}

    struct hasher
    {
        std::size_t operator() (Key const& v) const
        {
            switch (v.kind)
            {
            case kindInbound:
            case kindOutbound:
                return m_addr_hash (v.address);

            case kindUnlimited:
                return m_name_hash (v.name);

            default:
                assert(false);
            };

            return 0;
        }

    private:
        beast::uhash <> m_addr_hash;
        beast::uhash <> m_name_hash;
    };

    struct key_equal
    {
        bool operator() (Key const& lhs, Key const& rhs) const
        {
            if (lhs.kind != rhs.kind)
                return false;

            switch (lhs.kind)
            {
            case kindInbound:
            case kindOutbound:
                return lhs.address == rhs.address;

            case kindUnlimited:
                return lhs.name == rhs.name;

            default:
                assert(false);
            };

            return false;
        }

    private:
    };
};

}
}

#endif
