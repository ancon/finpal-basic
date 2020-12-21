//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_DUMMYSCHEDULER_H_INCLUDED
#define MTCHAIN_NODESTORE_DUMMYSCHEDULER_H_INCLUDED

#include <mtchain/nodestore/Scheduler.h>

namespace mtchain {
namespace NodeStore {

/** Simple NodeStore Scheduler that just peforms the tasks synchronously. */
class DummyScheduler : public Scheduler
{
public:
    DummyScheduler ();
    ~DummyScheduler ();
    void scheduleTask (Task& task) override;
    void scheduledTasksStopped ();
    void onFetch (FetchReport const& report) override;
    void onBatchWrite (BatchWriteReport const& report) override;
};

}
}

#endif
