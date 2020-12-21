//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_TYPES_H_INCLUDED
#define MTCHAIN_APP_PATHS_TYPES_H_INCLUDED

namespace mtchain {

// account id, issue.
using AccountIssue = std::pair <AccountID, Issue>;

// Map of account, issue to node index.
namespace path {

using NodeIndex = unsigned int;

}

using AccountIssueToNodeIndex = hash_map <AccountIssue, path::NodeIndex>;

} //

#endif
