//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef BEAST_CONTAINER_AGED_UNORDERED_MULTIMAP_H_INCLUDED
#define BEAST_CONTAINER_AGED_UNORDERED_MULTIMAP_H_INCLUDED

#include <mtchain/beast/container/detail/aged_unordered_container.h>

#include <chrono>
#include <functional>
#include <memory>

namespace beast {

template <
    class Key,
    class T,
    class Clock = std::chrono::steady_clock,
    class Hash = std::hash <Key>,
    class KeyEqual = std::equal_to <Key>,
    class Allocator = std::allocator <std::pair <Key const, T>>
>
using aged_unordered_multimap = detail::aged_unordered_container <
    true, true, Key, T, Clock, Hash, KeyEqual, Allocator>;

}

#endif
