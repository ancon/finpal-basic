//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/CollectorManager.h>
#include <memory>

namespace mtchain {

class CollectorManagerImp
    : public CollectorManager
{
public:
    beast::Journal m_journal;
    beast::insight::Collector::ptr m_collector;
    std::unique_ptr <beast::insight::Groups> m_groups;

    CollectorManagerImp (Section const& params,
        beast::Journal journal)
        : m_journal (journal)
    {
        std::string const& server  = get<std::string> (params, "server");

        if (server == "statsd")
        {
            beast::IP::Endpoint const address (beast::IP::Endpoint::from_string (
                get<std::string> (params, "address")));
            std::string const& prefix (get<std::string> (params, "prefix"));

            m_collector = beast::insight::StatsDCollector::New (address, prefix, journal);
        }
        else
        {
            m_collector = beast::insight::NullCollector::New ();
        }

        m_groups = beast::insight::make_Groups (m_collector);
    }

    ~CollectorManagerImp ()
    {
    }

    beast::insight::Collector::ptr const& collector () override
    {
        return m_collector;
    }

    beast::insight::Group::ptr const& group (std::string const& name) override
    {
        return m_groups->get (name);
    }
};

//------------------------------------------------------------------------------

CollectorManager::~CollectorManager ()
{
}

std::unique_ptr<CollectorManager> CollectorManager::New(Section const& params,
    beast::Journal journal)
{
    return std::make_unique<CollectorManagerImp>(params, journal);
}

}
