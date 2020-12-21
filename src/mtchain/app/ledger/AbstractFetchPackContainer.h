//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_ABSTRACTFETCHPACKCONTAINER_H_INCLUDED
#define MTCHAIN_APP_LEDGER_ABSTRACTFETCHPACKCONTAINER_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/basics/Blob.h>
#include <boost/optional.hpp>

namespace mtchain {

/** An interface facilitating retrieval of fetch packs without
    an application or ledgermaster object.
*/
class AbstractFetchPackContainer
{
public:
    virtual ~AbstractFetchPackContainer() = default;

    /** Retrieves partial ledger data of the coresponding hash from peers.`

        @param nodeHash The 256-bit hash of the data to fetch.
        @return `boost::none` if the hash isn't cached,
            otherwise, the hash associated data.
    */
    virtual boost::optional<Blob> getFetchPack(uint256 const& nodeHash) = 0;
};

} //

#endif
