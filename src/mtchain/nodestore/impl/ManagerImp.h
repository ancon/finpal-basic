//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_MANAGERIMP_H_INCLUDED
#define MTCHAIN_NODESTORE_MANAGERIMP_H_INCLUDED

#include <mtchain/nodestore/Manager.h>
#include <mutex>
#include <vector>

namespace mtchain {
namespace NodeStore {

class ManagerImp : public Manager
{
private:
    std::mutex mutex_;
    std::vector<Factory*> list_;

public:
    static
    ManagerImp&
    instance();

    static
    void
    missing_backend();

    ManagerImp();

    ~ManagerImp();

    Factory*
    find (std::string const& name);

    void
    insert (Factory& factory) override;

    void
    erase (Factory& factory) override;

    std::unique_ptr <Backend>
    make_Backend (
        Section const& parameters,
        Scheduler& scheduler,
        beast::Journal journal) override;

    std::unique_ptr <Database>
    make_Database (
        std::string const& name,
        Scheduler& scheduler,
        beast::Journal journal,
        int readThreads,
        Section const& backendParameters) override;

    std::unique_ptr <DatabaseRotating>
    make_DatabaseRotating (
        std::string const& name,
        Scheduler& scheduler,
        std::int32_t readThreads,
        std::shared_ptr <Backend> writableBackend,
        std::shared_ptr <Backend> archiveBackend,
        beast::Journal journal) override;
};

}
}

#endif
