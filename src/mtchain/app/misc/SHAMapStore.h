//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_SHAMAPSTORE_H_INCLUDED
#define MTCHAIN_APP_MISC_SHAMAPSTORE_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/core/Config.h>
#include <mtchain/nodestore/Manager.h>
#include <mtchain/nodestore/Scheduler.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/core/Stoppable.h>

namespace mtchain {

class TransactionMaster;

/**
 * class to create database, launch online delete thread, and
 * related sqlite databse
 */
class SHAMapStore
    : public Stoppable
{
public:
    struct Setup
    {
        bool standalone = false;
        std::uint32_t deleteInterval = 0;
        bool advisoryDelete = false;
        std::uint32_t ledgerHistory = 0;
        Section nodeDatabase;
        std::string databasePath;
        std::uint32_t deleteBatch = 100;
        std::uint32_t backOff = 100;
        std::int32_t ageThreshold = 60;
    };

    SHAMapStore (Stoppable& parent) : Stoppable ("SHAMapStore", parent) {}

    /** Called by LedgerMaster every time a ledger validates. */
    virtual void onLedgerClosed(std::shared_ptr<Ledger const> const& ledger) = 0;

    virtual void rendezvous() const = 0;

    virtual std::uint32_t clampFetchDepth (std::uint32_t fetch_depth) const = 0;

    virtual std::unique_ptr <NodeStore::Database> makeDatabase (
            std::string const& name, std::int32_t readThreads) = 0;

    /** Highest ledger that may be deleted. */
    virtual LedgerIndex setCanDelete (LedgerIndex canDelete) = 0;

    /** Whether advisory delete is enabled. */
    virtual bool advisoryDelete() const = 0;

    /** Last ledger which was copied during rotation of backends. */
    virtual LedgerIndex getLastRotated() = 0;

    /** Highest ledger that may be deleted. */
    virtual LedgerIndex getCanDelete() = 0;

    /** The number of files that are needed. */
    virtual int fdlimit() const = 0;
};

//------------------------------------------------------------------------------

SHAMapStore::Setup
setup_SHAMapStore(Config const& c);

std::unique_ptr<SHAMapStore>
make_SHAMapStore(
    Application& app,
    SHAMapStore::Setup const& s,
    Stoppable& parent,
    NodeStore::Scheduler& scheduler,
    beast::Journal journal,
    beast::Journal nodeStoreJournal,
    TransactionMaster& transactionMaster,
    BasicConfig const& conf);
}

#endif
