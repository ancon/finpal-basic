//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_UNITY_HYPERLEVELDB_H_INCLUDED
#define MTCHAIN_UNITY_HYPERLEVELDB_H_INCLUDED

#include <beast/Config.h>

#if ! BEAST_WIN32

#define MTCHAIN_HYPERLEVELDB_AVAILABLE 1

#include <hyperleveldb/hyperleveldb/cache.h>
#include <hyperleveldb/hyperleveldb/filter_policy.h>
#include <hyperleveldb/hyperleveldb/db.h>
#include <hyperleveldb/hyperleveldb/write_batch.h>

#else

#define MTCHAIN_HYPERLEVELDB_AVAILABLE 0

#endif

#endif
