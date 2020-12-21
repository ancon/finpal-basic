//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_VALIDATIONS_H_INCLUDED
#define MTCHAIN_APP_MISC_VALIDATIONS_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/protocol/Protocol.h>
#include <mtchain/protocol/STValidation.h>
#include <memory>
#include <vector>

namespace mtchain {

// VFALCO TODO rename and move these type aliases into the Validations interface

// nodes validating and highest node ID validating
using ValidationSet = hash_map<PublicKey, STValidation::pointer>;

using ValidationCounter = std::pair<int, NodeID>;
using LedgerToValidationCounter = hash_map<uint256, ValidationCounter>;

class Validations
{
public:
    virtual ~Validations() = default;

    virtual bool addValidation (STValidation::ref, std::string const& source) = 0;

    virtual bool current (STValidation::ref) = 0;

    virtual ValidationSet getValidations (uint256 const& ledger) = 0;

    virtual std::size_t getTrustedValidationCount (uint256 const& ledger) = 0;

    /** Returns fees reported by trusted validators in the given ledger. */
    virtual
    std::vector <std::uint64_t>
    fees (uint256 const& ledger, std::uint64_t base) = 0;

    virtual int getNodesAfter (uint256 const& ledger) = 0;
    virtual int getLoadRatio (bool overLoaded) = 0;

    virtual hash_set<PublicKey> getCurrentPublicKeys () = 0;

    // VFALCO TODO make a type alias for this ugly return value!
    virtual LedgerToValidationCounter getCurrentValidations (
        uint256 currentLedger, uint256 previousLedger,
        LedgerIndex cutoffBefore) = 0;

    /** Return the times of all validations for a particular ledger hash. */
    virtual std::vector<NetClock::time_point> getValidationTimes (
        uint256 const& ledger) = 0;

    virtual std::list <STValidation::pointer>
    getCurrentTrustedValidations () = 0;

    virtual void flush () = 0;

    virtual void sweep () = 0;
};

extern
std::unique_ptr<Validations>
make_Validations(Application& app);

} //

#endif
