//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/nodestore/DummyScheduler.h>

namespace mtchain {
namespace NodeStore {

DummyScheduler::DummyScheduler ()
{
}

DummyScheduler::~DummyScheduler ()
{
}

void
DummyScheduler::scheduleTask (Task& task)
{
    // Invoke the task synchronously.
    task.performScheduledTask();
}

void
DummyScheduler::scheduledTasksStopped ()
{
}

void
DummyScheduler::onFetch (const FetchReport& report)
{
}

void
DummyScheduler::onBatchWrite (const BatchWriteReport& report)
{
}

}
}
