//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_TUNING_H_INCLUDED
#define MTCHAIN_APP_PATHS_TUNING_H_INCLUDED

namespace mtchain {

int const CALC_NODE_DELIVER_MAX_LOOPS = 100;
int const CALC_NODE_DELIVER_MAX_LOOPS_MQ = 2000;
int const NODE_ADVANCE_MAX_LOOPS = 100;
int const PAYMENT_MAX_LOOPS = 1000;
int const PATHFINDER_HIGH_PRIORITY = 100000;
int const PATHFINDER_MAX_PATHS = 50;
int const PATHFINDER_MAX_COMPLETE_PATHS = 1000;
int const PATHFINDER_MAX_PATHS_FROM_SOURCE = 10;

} //

#endif
