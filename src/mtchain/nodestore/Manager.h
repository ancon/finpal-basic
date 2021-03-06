//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_MANAGER_H_INCLUDED
#define MTCHAIN_NODESTORE_MANAGER_H_INCLUDED

#include <mtchain/nodestore/Factory.h>
#include <mtchain/nodestore/DatabaseRotating.h>
#include <mtchain/basics/BasicConfig.h>
#include <mtchain/basics/Log.h>
#include <mtchain/beast/utility/Journal.h>

namespace mtchain {
namespace NodeStore {

/** Singleton for managing NodeStore factories and back ends. */
class Manager
{
public:
    /** Returns the instance of the manager singleton. */
    static
    Manager&
    instance();

    /** Add a factory. */
    virtual
    void
    insert (Factory& factory) = 0;

    /** Remove a factory. */
    virtual
    void
    erase (Factory& factory) = 0;

    /** Return a pointer to the matching factory if it exists.
        @param  name The name to match, performed case-insensitive.
        @return `nullptr` if a match was not found.
    */
    //virtual Factory* find (std::string const& name) const = 0;

    /** Create a backend. */
    virtual
    std::unique_ptr <Backend>
    make_Backend (Section const& parameters,
        Scheduler& scheduler, beast::Journal journal) = 0;

    /** Construct a NodeStore database.

        The parameters are key value pairs passed to the backend. The
        'type' key must exist, it defines the choice of backend. Most
        backends also require a 'path' field.

        Some choices for 'type' are:
            HyperLevelDB, LevelDBFactory, SQLite, MDB

        If the fastBackendParameter is omitted or empty, no ephemeral database
        is used. If the scheduler parameter is omited or unspecified, a
        synchronous scheduler is used which performs all tasks immediately on
        the caller's thread.

        @note If the database cannot be opened or created, an exception is thrown.

        @param name A diagnostic label for the database.
        @param scheduler The scheduler to use for performing asynchronous tasks.
        @param readThreads The number of async read threads to create
        @param backendParameters The parameter string for the persistent backend.
        @param fastBackendParameters [optional] The parameter string for the ephemeral backend.

        @return The opened database.
    */
    virtual
    std::unique_ptr <Database>
    make_Database (std::string const& name, Scheduler& scheduler,
        beast::Journal journal, int readThreads,
            Section const& backendParameters) = 0;

    virtual
    std::unique_ptr <DatabaseRotating>
    make_DatabaseRotating (std::string const& name,
        Scheduler& scheduler, std::int32_t readThreads,
            std::shared_ptr <Backend> writableBackend,
                std::shared_ptr <Backend> archiveBackend,
                    beast::Journal journal) = 0;
};

//------------------------------------------------------------------------------

/** Create a Backend. */
std::unique_ptr <Backend>
make_Backend (Section const& config,
    Scheduler& scheduler, beast::Journal journal);

}
}

#endif
