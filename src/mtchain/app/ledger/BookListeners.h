//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_BOOKLISTENERS_H_INCLUDED
#define MTCHAIN_APP_LEDGER_BOOKLISTENERS_H_INCLUDED

#include <mtchain/net/InfoSub.h>
#include <memory>
#include <mutex>

namespace mtchain {

/** Listen to public/subscribe messages from a book. */
class BookListeners
{
public:
    using pointer = std::shared_ptr<BookListeners>;

    BookListeners () {}

    void addSubscriber (InfoSub::ref sub);
    void removeSubscriber (std::uint64_t sub);
    void publish (Json::Value const& jvObj);

private:
    std::recursive_mutex mLock;

    hash_map<std::uint64_t, InfoSub::wptr> mListeners;
};

} //

#endif
