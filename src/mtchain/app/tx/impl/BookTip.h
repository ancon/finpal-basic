//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_BOOK_BOOKTIP_H_INCLUDED
#define MTCHAIN_APP_BOOK_BOOKTIP_H_INCLUDED

#include <mtchain/protocol/Quality.h>
#include <mtchain/ledger/View.h>
#include <mtchain/protocol/Indexes.h>

#include <functional>

namespace mtchain {

class Logs;

/** Iterates and consumes raw offers in an order book.
    Offers are presented from highest quality to lowest quality. This will
    return all offers present including missing, invalid, unfunded, etc.
*/
class BookTip
{
private:
    ApplyView& view_;
    bool m_valid;
    uint256 m_book;
    uint256 m_end;
    uint256 m_dir;
    uint256 m_index;
    std::shared_ptr<SLE> m_entry;
    Quality m_quality;

public:
    /** Create the iterator. */
    BookTip (ApplyView& view, Book const& book);

    uint256 const&
    dir() const noexcept
    {
        return m_dir;
    }

    uint256 const&
    index() const noexcept
    {
        return m_index;
    }

    Quality const&
    quality() const noexcept
    {
        return m_quality;
    }

    SLE::pointer const&
    entry() const noexcept
    {
        return m_entry;
    }

    /** Erases the current offer and advance to the next offer.
        Complexity: Constant
        @return `true` if there is a next offer
    */
    bool
    step (beast::Journal j);
};

}

#endif
