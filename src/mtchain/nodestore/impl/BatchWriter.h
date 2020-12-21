//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_BATCHWRITER_H_INCLUDED
#define MTCHAIN_NODESTORE_BATCHWRITER_H_INCLUDED

#include <mtchain/nodestore/Scheduler.h>
#include <mtchain/nodestore/Task.h>
#include <mtchain/nodestore/Types.h>
#include <condition_variable>
#include <mutex>

namespace mtchain {
namespace NodeStore {

/** Batch-writing assist logic.

    The batch writes are performed with a scheduled task. Use of the
    class it not required. A backend can implement its own write batching,
    or skip write batching if doing so yields a performance benefit.

    @see Scheduler
*/
class BatchWriter : private Task
{
public:
    /** This callback does the actual writing. */
    struct Callback
    {
        virtual void writeBatch (Batch const& batch) = 0;
    };

    /** Create a batch writer. */
    BatchWriter (Callback& callback, Scheduler& scheduler);

    /** Destroy a batch writer.

        Anything pending in the batch is written out before this returns.
    */
    ~BatchWriter ();

    /** Store the object.

        This will add to the batch and initiate a scheduled task to
        write the batch out.
    */
    void store (std::shared_ptr<NodeObject> const& object);

    /** Get an estimate of the amount of writing I/O pending. */
    int getWriteLoad ();

private:
    void performScheduledTask ();
    void writeBatch ();
    void waitForWriting ();

private:
    using LockType = std::recursive_mutex;
    using CondvarType = std::condition_variable_any;

    Callback& m_callback;
    Scheduler& m_scheduler;
    LockType mWriteMutex;
    CondvarType mWriteCondition;
    int mWriteLoad;
    bool mWritePending;
    Batch mWriteSet;
};

}
}

#endif
