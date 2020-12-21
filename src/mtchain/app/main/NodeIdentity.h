//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MAIN_NODEIDENTITY_H_INCLUDED
#define MTCHAIN_APP_MAIN_NODEIDENTITY_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <utility>

namespace mtchain {

/** The cryptographic credentials identifying this server instance. */
std::pair<PublicKey, SecretKey>
loadNodeIdentity (Application& app);

} //

#endif
