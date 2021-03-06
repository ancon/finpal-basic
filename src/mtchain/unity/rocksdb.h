//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_UNITY_ROCKSDB_H_INCLUDED
#define MTCHAIN_UNITY_ROCKSDB_H_INCLUDED

#include <mtchain/beast/core/Config.h>

#ifdef __clang_major__
#if __clang_major__ >= 7
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Winconsistent-missing-override"
# pragma clang diagnostic ignored "-Wuninitialized"
#endif
#endif

#ifndef MTCHAIN_ROCKSDB_AVAILABLE
#if BEAST_WIN32
# define ROCKSDB_PLATFORM_WINDOWS
#else
# define ROCKSDB_PLATFORM_POSIX
# if BEAST_MAC || BEAST_IOS
#  define OS_MACOSX 1
# elif BEAST_BSD
#  define OS_FREEBSD
# else
#  define OS_LINUX
# endif
#endif
# if BEAST_WIN32
#  define MTCHAIN_ROCKSDB_AVAILABLE 0
# else
#  if __cplusplus >= 201103L
#   define MTCHAIN_ROCKSDB_AVAILABLE 1
#  else
#   define MTCHAIN_ROCKSDB_AVAILABLE 0
#  endif
# endif
#endif

#if MTCHAIN_ROCKSDB_AVAILABLE
#define SNAPPY
//#include <rocksdb2/port/port_posix.h>
#include <rocksdb2/include/rocksdb/cache.h>
#include <rocksdb2/include/rocksdb/compaction_filter.h>
#include <rocksdb2/include/rocksdb/comparator.h>
#include <rocksdb2/include/rocksdb/db.h>
#include <rocksdb2/include/rocksdb/env.h>
#include <rocksdb2/include/rocksdb/filter_policy.h>
#include <rocksdb2/include/rocksdb/flush_block_policy.h>
#include <rocksdb2/include/rocksdb/iterator.h>
#include <rocksdb2/include/rocksdb/memtablerep.h>
#include <rocksdb2/include/rocksdb/merge_operator.h>
#include <rocksdb2/include/rocksdb/options.h>
#include <rocksdb2/include/rocksdb/perf_context.h>
#include <rocksdb2/include/rocksdb/slice.h>
#include <rocksdb2/include/rocksdb/slice_transform.h>
#include <rocksdb2/include/rocksdb/statistics.h>
#include <rocksdb2/include/rocksdb/status.h>
#include <rocksdb2/include/rocksdb/table.h>
#include <rocksdb2/include/rocksdb/table_properties.h>
#include <rocksdb2/include/rocksdb/transaction_log.h>
#include <rocksdb2/include/rocksdb/types.h>
#include <rocksdb2/include/rocksdb/universal_compaction.h>
#include <rocksdb2/include/rocksdb/write_batch.h>

#endif

#endif
