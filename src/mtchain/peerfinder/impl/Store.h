//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_STORE_H_INCLUDED
#define MTCHAIN_PEERFINDER_STORE_H_INCLUDED

namespace mtchain {
namespace PeerFinder {

/** Abstract persistence for PeerFinder data. */
class Store
{
public:
    virtual ~Store () { }

    // load the bootstrap cache
    using load_callback = std::function <void (beast::IP::Endpoint, int)>;
    virtual std::size_t load (load_callback const& cb) = 0;

    // save the bootstrap cache
    struct Entry
    {
        beast::IP::Endpoint endpoint;
        int valence;
    };
    virtual void save (std::vector <Entry> const& v) = 0;
};

}
}

#endif
