//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_FEEVOTE_H_INCLUDED
#define MTCHAIN_APP_MISC_FEEVOTE_H_INCLUDED

#include <mtchain/ledger/ReadView.h>
#include <mtchain/shamap/SHAMap.h>
#include <mtchain/app/misc/Validations.h>
#include <mtchain/basics/BasicConfig.h>
#include <mtchain/protocol/SystemParameters.h>

namespace mtchain {

/** Manager to process fee votes. */
class FeeVote
{
public:
    /** Fee schedule to vote for.
        During voting ledgers, the FeeVote logic will try to move towards
        these values when injecting fee-setting transactions.
        A default-constructed Setup contains recommended values.
    */
    struct Setup
    {
        /** The cost of a reference transaction in drops. */
        std::uint64_t reference_fee = 100;

        /** The cost of a reference transaction in fee units. */
        std::uint32_t const reference_fee_units = 100;

        /** The account reserve requirement in drops. */
        std::uint64_t account_reserve = 0.0001 * SYSTEM_CURRENCY_PARTS;

        /** The per-owned item reserve requirement in drops. */
        std::uint64_t owner_reserve = 0.0005 * SYSTEM_CURRENCY_PARTS;
    };

    virtual ~FeeVote () = default;

    /** Add local fee preference to validation.

        @param lastClosedLedger
        @param baseValidation
    */
    virtual
    void
    doValidation (std::shared_ptr<ReadView const> const& lastClosedLedger,
        STObject& baseValidation) = 0;

    /** Cast our local vote on the fee.

        @param lastClosedLedger
        @param initialPosition
    */
    virtual
    void
    doVoting (std::shared_ptr<ReadView const> const& lastClosedLedger,
        ValidationSet const& parentValidations,
            std::shared_ptr<SHAMap> const& initialPosition) = 0;
};

/** Build FeeVote::Setup from a config section. */
FeeVote::Setup
setup_FeeVote (Section const& section);

/** Create an instance of the FeeVote logic.
    @param setup The fee schedule to vote for.
    @param journal Where to log.
*/
std::unique_ptr <FeeVote>
make_FeeVote (FeeVote::Setup const& setup, beast::Journal journal);

} //

#endif
