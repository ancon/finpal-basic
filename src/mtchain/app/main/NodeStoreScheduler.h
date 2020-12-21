//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MAIN_NODESTORESCHEDULER_H_INCLUDED
#define MTCHAIN_APP_MAIN_NODESTORESCHEDULER_H_INCLUDED

#include <mtchain/nodestore/Scheduler.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/core/Stoppable.h>
#include <atomic>

namespace mtchain {

/** A NodeStore::Scheduler which uses the JobQueue and implements the Stoppable API. */
class NodeStoreScheduler
    : public NodeStore::Scheduler
    , public Stoppable
{
public:
    NodeStoreScheduler (Stoppable& parent);

    // VFALCO NOTE This is a temporary hack to solve the problem
    //             of circular dependency.
    //
    void setJobQueue (JobQueue& jobQueue);

    void onStop () override;
    void onChildrenStopped () override;
    void scheduleTask (NodeStore::Task& task) override;
    void onFetch (NodeStore::FetchReport const& report) override;
    void onBatchWrite (NodeStore::BatchWriteReport const& report) override;

private:
    void doTask (NodeStore::Task& task);

    JobQueue* m_jobQueue;
    std::atomic <int> m_taskCount;
};

} //

#endif
