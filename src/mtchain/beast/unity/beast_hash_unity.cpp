//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/beast/hash/xxhasher.h>
#include <mtchain/beast/hash/impl/spookyv2.cpp>

#if ! BEAST_NO_XXHASH
#include <mtchain/beast/hash/impl/xxhash.c>
#endif
#include <mtchain/beast/hash/impl/siphash.cpp>
