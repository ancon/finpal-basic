//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/nodestore/impl/ManagerImp.h>
#include <mtchain/nodestore/impl/DatabaseImp.h>
#include <mtchain/nodestore/impl/DatabaseRotatingImp.h>
#include <mtchain/basics/StringUtilities.h>
#include <beast/core/detail/ci_char_traits.hpp>
#include <memory>
#include <stdexcept>

namespace mtchain {
namespace NodeStore {

ManagerImp&
ManagerImp::instance()
{
    static ManagerImp _;
    return _;
}

void
ManagerImp::missing_backend()
{
    Throw<std::runtime_error> (
        "Your FinPal-core.cfg is missing a [node_db] entry, "
        "please see the FinPal-example.cfg file!"
        );
}

ManagerImp::ManagerImp()
{
}

ManagerImp::~ManagerImp()
{
}

std::unique_ptr <Backend>
ManagerImp::make_Backend (
    Section const& parameters,
    Scheduler& scheduler,
    beast::Journal journal)
{
    std::unique_ptr <Backend> backend;

    std::string const type (get<std::string>(parameters, "type"));

    if (! type.empty ())
    {
        Factory* const factory (find (type));

        if (factory != nullptr)
        {
            backend = factory->createInstance (
                NodeObject::keyBytes, parameters, scheduler, journal);
        }
        else
        {
            missing_backend ();
        }
    }
    else
    {
        missing_backend ();
    }

    return backend;
}

std::unique_ptr <Database>
ManagerImp::make_Database (
    std::string const& name,
    Scheduler& scheduler,
    beast::Journal journal,
    int readThreads,
    Section const& backendParameters)
{
    return std::make_unique <DatabaseImp> (
        name,
        scheduler,
        readThreads,
        make_Backend (
            backendParameters,
            scheduler,
            journal),
        journal);
}

std::unique_ptr <DatabaseRotating>
ManagerImp::make_DatabaseRotating (
        std::string const& name,
        Scheduler& scheduler,
        std::int32_t readThreads,
        std::shared_ptr <Backend> writableBackend,
        std::shared_ptr <Backend> archiveBackend,
        beast::Journal journal)
{
    return std::make_unique <DatabaseRotatingImp> (
        name,
        scheduler,
        readThreads,
        writableBackend,
        archiveBackend,
        journal);
}

Factory*
ManagerImp::find (std::string const& name)
{
    std::lock_guard<std::mutex> _(mutex_);
    auto const iter = std::find_if(list_.begin(), list_.end(),
        [&name](Factory* other)
        {
            return beast::detail::ci_equal(name, other->getName());
        } );
    if (iter == list_.end())
        return nullptr;
    return *iter;
}


void
ManagerImp::insert (Factory& factory)
{
    std::lock_guard<std::mutex> _(mutex_);
    list_.push_back(&factory);
}

void
ManagerImp::erase (Factory& factory)
{
    std::lock_guard<std::mutex> _(mutex_);
    auto const iter = std::find_if(list_.begin(), list_.end(),
        [&factory](Factory* other) { return other == &factory; });
    assert(iter != list_.end());
    list_.erase(iter);
}

//------------------------------------------------------------------------------

Manager&
Manager::instance()
{
    return ManagerImp::instance();
}

//------------------------------------------------------------------------------

std::unique_ptr <Backend>
make_Backend (Section const& config,
    Scheduler& scheduler, beast::Journal journal)
{
    return Manager::instance().make_Backend (
        config, scheduler, journal);
}

}
}
