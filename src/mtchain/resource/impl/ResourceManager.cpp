//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/resource/ResourceManager.h>
#include <mtchain/resource/impl/Logic.h>
#include <mtchain/basics/chrono.h>
#include <mtchain/basics/Log.h>
#include <mtchain/beast/core/CurrentThreadName.h>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace mtchain {
namespace Resource {

class ManagerImp : public Manager
{
private:
    beast::Journal journal_;
    Logic logic_;
    std::thread thread_;
    bool stop_ = false;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    ManagerImp (beast::insight::Collector::ptr const& collector,
        beast::Journal journal)
        : journal_ (journal)
        , logic_ (collector, stopwatch(), journal)
    {
        thread_ = std::thread {&ManagerImp::run, this};
    }

    ManagerImp () = delete;
    ManagerImp (ManagerImp const&) = delete;
    ManagerImp& operator= (ManagerImp const&) = delete;

    ~ManagerImp () override
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
            cond_.notify_one();
        }
        thread_.join();
    }

    Consumer newInboundEndpoint (beast::IP::Endpoint const& address) override
    {
        return logic_.newInboundEndpoint (address);
    }

    Consumer newOutboundEndpoint (beast::IP::Endpoint const& address) override
    {
        return logic_.newOutboundEndpoint (address);
    }

    Consumer newUnlimitedEndpoint (std::string const& name) override
    {
        return logic_.newUnlimitedEndpoint (name);
    }

    Gossip exportConsumers () override
    {
        return logic_.exportConsumers();
    }

    void importConsumers (
        std::string const& origin, Gossip const& gossip) override
    {
        logic_.importConsumers (origin, gossip);
    }

    //--------------------------------------------------------------------------

    Json::Value getJson () override
    {
        return logic_.getJson ();
    }

    Json::Value getJson (int threshold) override
    {
        return logic_.getJson (threshold);
    }

    //--------------------------------------------------------------------------

    void onWrite (beast::PropertyStream::Map& map) override
    {
        logic_.onWrite (map);
    }

    //--------------------------------------------------------------------------

private:
    void run ()
    {
        beast::setCurrentThreadName ("Resource::Manager");
        for(;;)
        {
            logic_.periodicActivity();
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait_for(lock, std::chrono::seconds(1));
            if (stop_)
                break;
        }
    }
};

//------------------------------------------------------------------------------

Manager::Manager ()
    : beast::PropertyStream::Source ("resource")
{
}

Manager::~Manager ()
{
}

//------------------------------------------------------------------------------

std::unique_ptr <Manager> make_Manager (
    beast::insight::Collector::ptr const& collector,
        beast::Journal journal)
{
    return std::make_unique <ManagerImp> (collector, journal);
}

}
}
