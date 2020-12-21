//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================
#include <BeastConfig.h>

// Core soci
#include <core/connection-parameters.cpp>
#include <core/connection-pool.cpp>
#include <core/error.cpp>
#include <core/into-type.cpp>
#include <core/once-temp-type.cpp>
#include <core/prepare-temp-type.cpp>
#include <core/procedure.cpp>
#include <core/ref-counted-prepare-info.cpp>
#include <core/ref-counted-statement.cpp>
#include <core/row.cpp>
#include <core/rowid.cpp>
#include <core/session.cpp>
#include <core/soci-simple.cpp>
#include <core/statement.cpp>
#include <core/transaction.cpp>
#include <core/use-type.cpp>
#include <core/values.cpp>

#include <backends/sqlite3/common.cpp>
#include <backends/sqlite3/error.cpp>
#include <backends/sqlite3/factory.cpp>
#include <backends/sqlite3/row-id.cpp>
#include <backends/sqlite3/session.cpp>
#include <backends/sqlite3/standard-into-type.cpp>
#include <backends/sqlite3/standard-use-type.cpp>
#include <backends/sqlite3/statement.cpp>
#include <backends/sqlite3/vector-into-type.cpp>
#include <backends/sqlite3/vector-use-type.cpp>

#include <core/blob.cpp>
#include <backends/sqlite3/blob.cpp>
