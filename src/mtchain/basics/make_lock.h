//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_MAKE_LOCK_H_INCLUDED
#define MTCHAIN_BASICS_MAKE_LOCK_H_INCLUDED

#include <mutex>
#include <utility>

namespace mtchain {

template <class Mutex, class ...Args>
inline
std::unique_lock<Mutex>
make_lock(Mutex& mutex, Args&&... args)
{
    return std::unique_lock<Mutex>(mutex, std::forward<Args>(args)...);
}

} //

#endif
