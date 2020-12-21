//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_CURSOR_EFFECTIVERATE_H_INCLUDED
#define MTCHAIN_APP_PATHS_CURSOR_EFFECTIVERATE_H_INCLUDED

#include <mtchain/protocol/AccountID.h>
#include <mtchain/protocol/Issue.h>
#include <mtchain/protocol/Rate.h>
#include <boost/optional.hpp>

namespace mtchain {
namespace path {

Rate
effectiveRate(
    Issue const& issue,
    AccountID const& account1,
    AccountID const& account2,
    boost::optional<Rate> const& rate);

} // path
} //

#endif
