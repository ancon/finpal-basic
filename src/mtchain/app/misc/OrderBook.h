//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_ORDERBOOK_H_INCLUDED
#define MTCHAIN_APP_MISC_ORDERBOOK_H_INCLUDED

namespace mtchain {

/** Describes a serialized ledger entry for an order book. */
class OrderBook
{
public:
    using pointer = std::shared_ptr <OrderBook>;
    using ref = std::shared_ptr <OrderBook> const&;
    using List = std::vector<pointer>;

    /** Construct from a currency specification.

        @param index ???
        @param book in and out currency/issuer pairs.
    */
    // VFALCO NOTE what is the meaning of the index parameter?
    OrderBook (uint256 const& base, Book const& book)
            : mBookBase(base), mBook(book)
    {
    }

    uint256 const& getBookBase () const
    {
        return mBookBase;
    }

    Book const& book() const
    {
        return mBook;
    }

    Currency const& getCurrencyIn () const
    {
        return mBook.in.currency;
    }

    Currency const& getCurrencyOut () const
    {
        return mBook.out.currency;
    }

    AccountID const& getIssuerIn () const
    {
        return mBook.in.account;
    }

    AccountID const& getIssuerOut () const
    {
        return mBook.out.account;
    }

private:
    uint256 const mBookBase;
    Book const mBook;
};

} //

#endif
