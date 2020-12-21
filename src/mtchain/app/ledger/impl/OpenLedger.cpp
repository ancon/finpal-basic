//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/OpenLedger.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/TxQ.h>
#include <mtchain/app/tx/apply.h>
#include <mtchain/ledger/CachedView.h>
#include <mtchain/protocol/Feature.h>
#include <boost/range/adaptor/transformed.hpp>

namespace mtchain {

OpenLedger::OpenLedger(std::shared_ptr<
    Ledger const> const& ledger,
        CachedSLEs& cache,
            beast::Journal journal)
    : j_ (journal)
    , cache_ (cache)
    , current_ (create(ledger->rules(), ledger))
{
}

bool
OpenLedger::empty() const
{
    std::lock_guard<
        std::mutex> lock(modify_mutex_);
    return current_->txCount() == 0;
}

std::shared_ptr<OpenView const>
OpenLedger::current() const
{
    std::lock_guard<
        std::mutex> lock(
            current_mutex_);
    return current_;
}

bool
OpenLedger::modify (modify_type const& f)
{
    std::lock_guard<
        std::mutex> lock1(modify_mutex_);
    auto next = std::make_shared<
        OpenView>(*current_);
    auto const changed = f(*next, j_);
    if (changed)
    {
        std::lock_guard<
            std::mutex> lock2(
                current_mutex_);
        current_ = std::move(next);
    }
    return changed;
}

void
OpenLedger::accept(Application& app, Rules const& rules,
    std::shared_ptr<Ledger const> const& ledger,
        OrderedTxs const& locals, bool retriesFirst,
            OrderedTxs& retries, ApplyFlags flags,
                std::string const& suffix,
                    modify_type const& f)
{
    JLOG(j_.trace()) <<
        "accept ledger " << ledger->seq() << " " << suffix;
    auto next = create(rules, ledger);
    if (retriesFirst)
    {
        // Handle disputed tx, outside lock
        using empty =
            std::vector<std::shared_ptr<
                STTx const>>;
        apply (app, *next, *ledger, empty{},
            retries, flags, j_);
    }
    // Block calls to modify, otherwise
    // new tx going into the open ledger
    // would get lost.
    std::lock_guard<
        std::mutex> lock1(modify_mutex_);
    // Apply tx from the current open view
    if (! current_->txs.empty())
        apply (app, *next, *ledger,
            boost::adaptors::transform(
                current_->txs,
            [](std::pair<std::shared_ptr<
                STTx const>, std::shared_ptr<
                    STObject const>> const& p)
            {
                return p.first;
            }),
                retries, flags, j_);
    // Call the modifier
    if (f)
        f(*next, j_);
    // Apply local tx
    for (auto const& item : locals)
        app.getTxQ().apply(app, *next,
            item.second, flags, j_);
    // Switch to the new open view
    std::lock_guard<
        std::mutex> lock2(current_mutex_);
    current_ = std::move(next);
}

//------------------------------------------------------------------------------

std::shared_ptr<OpenView>
OpenLedger::create (Rules const& rules,
    std::shared_ptr<Ledger const> const& ledger)
{
    return std::make_shared<OpenView>(
        open_ledger, rules, std::make_shared<
            CachedLedger const>(ledger,
                cache_));
}

auto
OpenLedger::apply_one (Application& app, OpenView& view,
    std::shared_ptr<STTx const> const& tx,
        bool retry, ApplyFlags flags,
            beast::Journal j) -> Result
{
    if (retry)
        flags = flags | tapRETRY;
    auto const result = mtchain::apply(
        app, view, *tx, flags, j);
    if (result.second)
        return Result::success;
    if (isTefFailure (result.first) ||
        isTemMalformed (result.first) ||
            isTelLocal (result.first))
        return Result::failure;
    return Result::retry;
}

//------------------------------------------------------------------------------

std::string
debugTxstr (std::shared_ptr<STTx const> const& tx)
{
    std::stringstream ss;
    ss << tx->getTransactionID();
    return ss.str().substr(0, 4);
}

std::string
debugTostr (OrderedTxs const& set)
{
    std::stringstream ss;
    for(auto const& item : set)
        ss << debugTxstr(item.second) << ", ";
    return ss.str();
}

std::string
debugTostr (SHAMap const& set)
{
    std::stringstream ss;
    for (auto const& item : set)
    {
        try
        {
            SerialIter sit(item.slice());
            auto const tx = std::make_shared<
                STTx const>(sit);
            ss << debugTxstr(tx) << ", ";
        }
        catch(std::exception const&)
        {
            ss << "THRO, ";
        }
    }
    return ss.str();
}

std::string
debugTostr (std::shared_ptr<ReadView const> const& view)
{
    std::stringstream ss;
    for(auto const& item : view->txs)
        ss << debugTxstr(item.first) << ", ";
    return ss.str();
}

} //
