//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_CORE_COROINL_H_INCLUDED
#define MTCHAIN_CORE_COROINL_H_INCLUDED

namespace mtchain {

template <class F>
JobQueue::Coro::
Coro(Coro_create_t, JobQueue& jq, JobType type,
    std::string const& name, F&& f)
    : jq_(jq)
    , type_(type)
    , name_(name)
    , running_(false)
    , coro_(
        [this, fn = std::forward<F>(f)]
        (boost::coroutines::asymmetric_coroutine<void>::push_type& do_yield)
        {
            yield_ = &do_yield;
            yield();
            fn(shared_from_this());
#ifndef NDEBUG
            finished_ = true;
#endif
        }, boost::coroutines::attributes (1024 * 1024))
{
}

inline
JobQueue::Coro::
~Coro()
{
    assert(finished_);
}

inline
void
JobQueue::Coro::
yield() const
{
    {
        std::lock_guard<std::mutex> lock(jq_.m_mutex);
        ++jq_.nSuspend_;
    }
    (*yield_)();
}

inline
void
JobQueue::Coro::
post()
{
    {
        std::lock_guard<std::mutex> lk(mutex_run_);
        running_ = true;
    }

    // sp keeps 'this' alive
    jq_.addJob(type_, name_,
        [this, sp = shared_from_this()](Job&)
        {
            {
                std::lock_guard<std::mutex> lock(jq_.m_mutex);
                --jq_.nSuspend_;
            }
            auto saved = detail::getLocalValues().release();
            detail::getLocalValues().reset(&lvs_);
            std::lock_guard<std::mutex> lock(mutex_);
            coro_();
            detail::getLocalValues().release();
            detail::getLocalValues().reset(saved);
            std::lock_guard<std::mutex> lk(mutex_run_);
            running_ = false;
            cv_.notify_all();
        });
}

inline
void
JobQueue::Coro::
join()
{
    std::unique_lock<std::mutex> lk(mutex_run_);
    cv_.wait(lk,
        [this]()
        {
            return running_ == false;
        });
}

} //

#endif
