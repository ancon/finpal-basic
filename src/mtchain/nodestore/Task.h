//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_NODESTORE_TASK_H_INCLUDED
#define MTCHAIN_NODESTORE_TASK_H_INCLUDED

namespace mtchain {
namespace NodeStore {

/** Derived classes perform scheduled tasks. */
struct Task
{
    virtual ~Task() = default;

    /** Performs the task.
        The call may take place on a foreign thread.
    */
    virtual void performScheduledTask() = 0;
};

}
}

#endif
