//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/OrderBookDB.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

OrderBookDB::OrderBookDB (Application& app, Stoppable& parent)
    : Stoppable ("OrderBookDB", parent)
    , app_ (app)
    , mSeq (0)
    , j_ (app.journal ("OrderBookDB"))
{
}

void OrderBookDB::invalidate ()
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    mSeq = 0;
}

void OrderBookDB::setup(
    std::shared_ptr<ReadView const> const& ledger)
{
    {
        std::lock_guard <std::recursive_mutex> sl (mLock);
        auto seq = ledger->info().seq;

        // Do a full update every 256 ledgers
        if (mSeq != 0)
        {
            if (seq == mSeq)
                return;
            if ((seq > mSeq) && ((seq - mSeq) < 256))
                return;
            if ((seq < mSeq) && ((mSeq - seq) < 16))
                return;
        }

        JLOG (j_.debug())
            << "Advancing from " << mSeq << " to " << seq;

        mSeq = seq;
    }

    if (app_.config().PATH_SEARCH_MAX == 0)
    {
        // nothing to do
    }
    else if (app_.config().standalone())
        update(ledger);
    else
        app_.getJobQueue().addJob(
            jtUPDATE_PF, "OrderBookDB::update",
            [this, ledger] (Job&) { update(ledger); });
}

void OrderBookDB::update(
    std::shared_ptr<ReadView const> const& ledger)
{
    hash_set< uint256 > seen;
    OrderBookDB::IssueToOrderBook destMap;
    OrderBookDB::IssueToOrderBook sourceMap;
    hash_set< Issue > MBooks;

    JLOG (j_.debug()) << "OrderBookDB::update>";

    if (app_.config().PATH_SEARCH_MAX == 0)
    {
        // pathfinding has been disabled
        return;
    }

    // walk through the entire ledger looking for orderbook entries
    int books = 0;

    try
    {
        for(auto& sle : ledger->sles)
        {
            if (sle->getType () == ltDIR_NODE &&
                sle->isFieldPresent (sfExchangeRate) &&
                sle->getFieldH256 (sfRootIndex) == sle->key())
            {
                Book book;
                book.in.currency.copyFrom(sle->getFieldH160(
                    sfTakerPaysCurrency));
                book.in.account.copyFrom(sle->getFieldH160 (
                    sfTakerPaysIssuer));
                book.out.account.copyFrom(sle->getFieldH160(
                    sfTakerGetsIssuer));
                book.out.currency.copyFrom (sle->getFieldH160(
                    sfTakerGetsCurrency));

                uint256 index = getBookBase (book);
                if (seen.insert (index).second)
                {
                    auto orderBook = std::make_shared<OrderBook> (index, book);
                    sourceMap[book.in].push_back (orderBook);
                    destMap[book.out].push_back (orderBook);
                    if (isM(book.out))
                        MBooks.insert(book.in);
                    ++books;
                }
            }
        }
    }
    catch (const SHAMapMissingNode&)
    {
        JLOG (j_.info())
            << "OrderBookDB::update encountered a missing node";
        std::lock_guard <std::recursive_mutex> sl (mLock);
        mSeq = 0;
        return;
    }

    JLOG (j_.debug())
        << "OrderBookDB::update< " << books << " books found";
    {
        std::lock_guard <std::recursive_mutex> sl (mLock);

        mMBooks.swap(MBooks);
        mSourceMap.swap(sourceMap);
        mDestMap.swap(destMap);
    }
    app_.getLedgerMaster().newOrderBookDB();
}

void OrderBookDB::addOrderBook(Book const& book)
{
    bool toM = isM (book.out);
    std::lock_guard <std::recursive_mutex> sl (mLock);

    if (toM)
    {
        // We don't want to search through all the to-M or from-M order
        // books!
        for (auto ob: mSourceMap[book.in])
        {
            if (isM (ob->getCurrencyOut ())) // also to M
                return;
        }
    }
    else
    {
        for (auto ob: mDestMap[book.out])
        {
            if (ob->getCurrencyIn() == book.in.currency &&
                ob->getIssuerIn() == book.in.account)
            {
                return;
            }
        }
    }
    uint256 index = getBookBase(book);
    auto orderBook = std::make_shared<OrderBook> (index, book);

    mSourceMap[book.in].push_back (orderBook);
    mDestMap[book.out].push_back (orderBook);
    if (toM)
        mMBooks.insert(book.in);
}

// return list of all orderbooks that want this issuerID and currencyID
OrderBook::List OrderBookDB::getBooksByTakerPays (Issue const& issue)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    auto it = mSourceMap.find (issue);
    return it == mSourceMap.end () ? OrderBook::List() : it->second;
}

int OrderBookDB::getBookSize(Issue const& issue) {
    std::lock_guard <std::recursive_mutex> sl (mLock);
    auto it = mSourceMap.find (issue);
    return it == mSourceMap.end () ? 0 : it->second.size();
}

bool OrderBookDB::isBookToM(Issue const& issue)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    return mMBooks.count(issue) > 0;
}

BookListeners::pointer OrderBookDB::makeBookListeners (Book const& book)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);
    auto ret = getBookListeners (book);

    if (!ret)
    {
        ret = std::make_shared<BookListeners> ();

        mListeners [book] = ret;
        assert (getBookListeners (book) == ret);
    }

    return ret;
}

BookListeners::pointer OrderBookDB::getBookListeners (Book const& book)
{
    BookListeners::pointer ret;
    std::lock_guard <std::recursive_mutex> sl (mLock);

    auto it0 = mListeners.find (book);
    if (it0 != mListeners.end ())
        ret = it0->second;

    return ret;
}

// Based on the meta, send the meta to the streams that are listening.
// We need to determine which streams a given meta effects.
void OrderBookDB::processTxn (
    std::shared_ptr<ReadView const> const& ledger,
        const AcceptedLedgerTx& alTx, Json::Value const& jvObj)
{
    std::lock_guard <std::recursive_mutex> sl (mLock);

    if (alTx.getResult () == tesSUCCESS)
    {
        // Check if this is an offer or an offer cancel or a payment that
        // consumes an offer.
        // Check to see what the meta looks like.
        for (auto& node : alTx.getMeta ()->getNodes ())
        {
            try
            {
                if (node.getFieldU16 (sfLedgerEntryType) == ltOFFER)
                {
                    SField const* field = nullptr;

                    // We need a field that contains the TakerGets and TakerPays
                    // parameters.
                    if (node.getFName () == sfModifiedNode)
                        field = &sfPreviousFields;
                    else if (node.getFName () == sfCreatedNode)
                        field = &sfNewFields;
                    else if (node.getFName () == sfDeletedNode)
                        field = &sfFinalFields;

                    if (field)
                    {
                        auto data = dynamic_cast<const STObject*> (
                            node.peekAtPField (*field));

                        if (data &&
                            data->isFieldPresent (sfTakerPays) &&
                            data->isFieldPresent (sfTakerGets))
                        {
                            // determine the OrderBook
                            auto listeners = getBookListeners (
                                {data->getFieldAmount (sfTakerGets).issue(),
                                 data->getFieldAmount (sfTakerPays).issue()});

                            if (listeners)
                                listeners->publish (jvObj);
                        }
                    }
                }
            }
            catch (std::exception const&)
            {
                JLOG (j_.info())
                    << "Fields not found in OrderBookDB::processTxn";
            }
        }
    }
}

} //
