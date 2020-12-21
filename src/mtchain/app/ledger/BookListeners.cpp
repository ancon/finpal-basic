//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/OrderBookDB.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/json/to_string.h>

namespace mtchain {

void BookListeners::addSubscriber (InfoSub::ref sub)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    mListeners[sub->getSeq ()] = sub;
}

void BookListeners::removeSubscriber (std::uint64_t seq)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    mListeners.erase (seq);
}

void BookListeners::publish (Json::Value const& jvObj)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    auto it = mListeners.cbegin ();

    while (it != mListeners.cend ())
    {
        InfoSub::pointer p = it->second.lock ();

        if (p)
        {
            p->send (jvObj, true);
            ++it;
        }
        else
            it = mListeners.erase (it);
    }
}

} //
