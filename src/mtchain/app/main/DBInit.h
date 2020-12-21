//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_DATA_DBINIT_H_INCLUDED
#define MTCHAIN_APP_DATA_DBINIT_H_INCLUDED

namespace mtchain {

// VFALCO TODO Tidy these up into a class with functions and return types.
extern const char* TxnDBInit[];
extern const char* LedgerDBInit[];
extern const char* WalletDBInit[];

// VFALCO TODO Figure out what these counts are for
extern int TxnDBCount;
extern int LedgerDBCount;
extern int WalletDBCount;

} //

#endif
