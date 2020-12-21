//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_SIM_MESSAGE_H_INCLUDED
#define MTCHAIN_PEERFINDER_SIM_MESSAGE_H_INCLUDED

namespace mtchain {
namespace PeerFinder {
namespace Sim {

class Message
{
public:
    explicit Message (Endpoints const& endpoints)
        : m_payload (endpoints)
        { }
    Endpoints const& payload () const
        { return m_payload; }
private:
    Endpoints m_payload;
};

}
}
}

#endif
