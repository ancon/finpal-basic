//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/NodeStoreScheduler.h>
#include <cassert>

namespace mtchain {

NodeStoreScheduler::NodeStoreScheduler (Stoppable& parent)
    : Stoppable ("NodeStoreScheduler", parent)
    , m_jobQueue (nullptr)
    , m_taskCount (0)
{
}

void NodeStoreScheduler::setJobQueue (JobQueue& jobQueue)
{
    m_jobQueue = &jobQueue;
}

void NodeStoreScheduler::onStop ()
{
}

void NodeStoreScheduler::onChildrenStopped ()
{
    assert (m_taskCount == 0);
    stopped ();
}

void NodeStoreScheduler::scheduleTask (NodeStore::Task& task)
{
    ++m_taskCount;
    m_jobQueue->addJob (
        jtWRITE,
        "NodeObject::store",
        [this, &task] (Job&) { doTask(task); });
}

void NodeStoreScheduler::doTask (NodeStore::Task& task)
{
    task.performScheduledTask ();
    if ((--m_taskCount == 0) && isStopping())
        stopped();
}

void NodeStoreScheduler::onFetch (NodeStore::FetchReport const& report)
{
    if (report.wentToDisk)
        m_jobQueue->addLoadEvents (
            report.isAsync ? jtNS_ASYNC_READ : jtNS_SYNC_READ,
                1, report.elapsed);
}

void NodeStoreScheduler::onBatchWrite (NodeStore::BatchWriteReport const& report)
{
    m_jobQueue->addLoadEvents (jtNS_WRITE,
        report.writeCount, report.elapsed);
}

} //
