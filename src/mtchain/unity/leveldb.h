//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_UNITY_LEVELDB_H_INCLUDED
#define MTCHAIN_UNITY_LEVELDB_H_INCLUDED

#ifndef MTCHAIN_LEVELDB_AVAILABLE
# define MTCHAIN_LEVELDB_AVAILABLE 1
#endif

#if MTCHAIN_LEVELDB_AVAILABLE

#define SNAPPY
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#endif

#endif
