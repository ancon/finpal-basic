//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_LEDGER_VIEW_H_INCLUDED
#define MTCHAIN_LEDGER_VIEW_H_INCLUDED

#include <mtchain/ledger/ApplyView.h>
#include <mtchain/ledger/OpenView.h>
#include <mtchain/ledger/RawView.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/Protocol.h>
#include <mtchain/protocol/Rate.h>
#include <mtchain/protocol/Serializer.h>
#include <mtchain/protocol/STLedgerEntry.h>
#include <mtchain/protocol/STObject.h>
#include <mtchain/protocol/STTx.h>
#include <mtchain/protocol/TER.h>
#include <mtchain/core/Config.h>
#include <mtchain/beast/utility/Journal.h>
#include <boost/optional.hpp>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <vector>

namespace mtchain {

//------------------------------------------------------------------------------
//
// Observers
//
//------------------------------------------------------------------------------

/** Controls the treatment of frozen account balances */
enum FreezeHandling
{
    fhIGNORE_FREEZE,
    fhZERO_IF_FROZEN
};

bool
isGlobalFrozen (ReadView const& view,
    AccountID const& issuer);

// Returns the amount an account can spend without going into debt.
//
// <-- saAmount: amount of currency held by account. May be negative.
STAmount
accountHolds (ReadView const& view,
    AccountID const& account, Currency const& currency,
        AccountID const& issuer, FreezeHandling zeroIfFrozen,
              beast::Journal j);

STAmount
accountFunds (ReadView const& view, AccountID const& id,
    STAmount const& saDefault, FreezeHandling freezeHandling,
        beast::Journal j);

/** Iterate all items in an account's owner directory. */
void
forEachItem (ReadView const& view, AccountID const& id,
    std::function<void (std::shared_ptr<SLE const> const&)> f);

/** Iterate all items after an item in an owner directory.
    @param after The key of the item to start after
    @param hint The directory page containing `after`
    @param limit The maximum number of items to return
    @return `false` if the iteration failed
*/
bool
forEachItemAfter (ReadView const& view, AccountID const& id,
    uint256 const& after, std::uint64_t const hint,
        unsigned int limit, std::function<
            bool (std::shared_ptr<SLE const> const&)> f);

Rate transferRate (ReadView const& view, AccountID const& issuer);
Rate transferRate (ReadView const& view, AccountID const& issuer, Currency const& currency);

/** Returns `true` if the directory is empty
    @param key The key of the directory
*/
bool
dirIsEmpty (ReadView const& view,
    Keylet const& k);

// Return the first entry and advance uDirEntry.
// <-- true, if had a next entry.
// VFALCO Fix these clumsy routines with an iterator
bool
cdirFirst (ReadView const& view,
    uint256 const& uRootIndex,  // --> Root of directory.
    std::shared_ptr<SLE const>& sleNode,      // <-> current node
    unsigned int& uDirEntry,    // <-- next entry
    uint256& uEntryIndex,       // <-- The entry, if available. Otherwise, zero.
    beast::Journal j);

// Return the current entry and advance uDirEntry.
// <-- true, if had a next entry.
// VFALCO Fix these clumsy routines with an iterator
bool
cdirNext (ReadView const& view,
    uint256 const& uRootIndex,  // --> Root of directory
    std::shared_ptr<SLE const>& sleNode,      // <-> current node
    unsigned int& uDirEntry,    // <-> next entry
    uint256& uEntryIndex,       // <-- The entry, if available. Otherwise, zero.
    beast::Journal j);

// Return the list of enabled amendments
std::set <uint256>
getEnabledAmendments (ReadView const& view);

// Return a map of amendments that have achieved majority
using majorityAmendments_t = std::map <uint256, NetClock::time_point>;
majorityAmendments_t
getMajorityAmendments (ReadView const& view);

/** Return the hash of a ledger by sequence.
    The hash is retrieved by looking up the "skip list"
    in the passed ledger. As the skip list is limited
    in size, if the requested ledger sequence number is
    out of the range of ledgers represented in the skip
    list, then boost::none is returned.
    @return The hash of the ledger with the
            given sequence number or boost::none.
*/
boost::optional<uint256>
hashOfSeq (ReadView const& ledger, LedgerIndex seq,
    beast::Journal journal);

/** Find a ledger index from which we could easily get the requested ledger

    The index that we return should meet two requirements:
        1) It must be the index of a ledger that has the hash of the ledger
            we are looking for. This means that its sequence must be equal to
            greater than the sequence that we want but not more than 256 greater
            since each ledger contains the hashes of the 256 previous ledgers.

        2) Its hash must be easy for us to find. This means it must be 0 mod 256
            because every such ledger is permanently enshrined in a LedgerHashes
            page which we can easily retrieve via the skip list.
*/
inline
LedgerIndex
getCandidateLedger (LedgerIndex requested)
{
    return (requested + 255) & (~255);
}

/** Return false if the test ledger is provably incompatible
    with the valid ledger, that is, they could not possibly
    both be valid. Use the first form if you have both ledgers,
    use the second form if you have not acquired the valid ledger yet
*/
bool areCompatible (ReadView const& validLedger, ReadView const& testLedger,
    beast::Journal::Stream& s, const char* reason);

bool areCompatible (uint256 const& validHash, LedgerIndex validIndex,
    ReadView const& testLedger, beast::Journal::Stream& s, const char* reason);

//------------------------------------------------------------------------------
//
// Modifiers
//
//------------------------------------------------------------------------------

/** Adjust the owner count up or down. */
void
adjustOwnerCount (ApplyView& view,
    std::shared_ptr<SLE> const& sle,
        int amount, beast::Journal j);

// Return the first entry and advance uDirEntry.
// <-- true, if had a next entry.
// VFALCO Fix these clumsy routines with an iterator
bool
dirFirst (ApplyView& view,
    uint256 const& uRootIndex,  // --> Root of directory.
    std::shared_ptr<SLE>& sleNode,      // <-> current node
    unsigned int& uDirEntry,    // <-- next entry
    uint256& uEntryIndex,       // <-- The entry, if available. Otherwise, zero.
    beast::Journal j);

// Return the current entry and advance uDirEntry.
// <-- true, if had a next entry.
// VFALCO Fix these clumsy routines with an iterator
bool
dirNext (ApplyView& view,
    uint256 const& uRootIndex,  // --> Root of directory
    std::shared_ptr<SLE>& sleNode,      // <-> current node
    unsigned int& uDirEntry,    // <-> next entry
    uint256& uEntryIndex,       // <-- The entry, if available. Otherwise, zero.
    beast::Journal j);

std::function<void (SLE::ref)>
describeOwnerDir(AccountID const& account);

// <--     uNodeDir: For deletion, present to make dirDelete efficient.
// -->   uRootIndex: The index of the base of the directory.  Nodes are based off of this.
// --> uLedgerIndex: Value to add to directory.
// Only append. This allow for things that watch append only structure to just monitor from the last node on ward.
// Within a node with no deletions order of elements is sequential.  Otherwise, order of elements is random.

/** Add an entry to directory, creating the directory if necessary

    @param uNodeDir node of entry - makes deletion efficient
    @param uRootIndex The index of the base of the directory.
                      Nodes are based off of this.
    @param uLedgerIndex Value to add to directory.

    @return a pair containing a code indicating success or
            failure, and if successful, a boolean indicating
            whether the directory was just created.
*/
std::pair<TER, bool>
dirAdd (ApplyView& view,
    std::uint64_t&                      uNodeDir,      // Node of entry.
    Keylet const&                       uRootIndex,
    uint256 const&                      uLedgerIndex,
    std::function<void (SLE::ref)>      fDescriber,
    beast::Journal j);

TER
dirDelete (ApplyView& view,
    const bool           bKeepRoot,
    const std::uint64_t& uNodeDir,      // Node item is mentioned in.
    uint256 const&       uRootIndex,
    uint256 const&       uLedgerIndex,  // Item being deleted
    const bool           bStable,
    const bool           bSoft,
    beast::Journal j);

// VFALCO NOTE Both STAmount parameters should just
//             be "Amount", a unit-less number.
//
/** Create a trust line

    This can set an initial balance.
*/
TER
trustCreate (ApplyView& view,
    const bool      bSrcHigh,
    AccountID const&  uSrcAccountID,
    AccountID const&  uDstAccountID,
    uint256 const&  uIndex,             // --> mtchain state entry
    SLE::ref        sleAccount,         // --> the account being set.
    const bool      bAuth,              // --> authorize account.
    const bool      bNoMTChain,          // --> others cannot mtchain through
    const bool      bFreeze,            // --> funds cannot leave
    STAmount const& saBalance,          // --> balance of account being set.
                                        // Issuer should be noAccount()
    STAmount const& saLimit,            // --> limit for account being set.
                                        // Issuer should be the account being set.
    std::uint32_t uSrcQualityIn,
    std::uint32_t uSrcQualityOut,
    beast::Journal j);

TER
trustDelete (ApplyView& view,
    std::shared_ptr<SLE> const& sleMtchainState,
        AccountID const& uLowAccountID,
            AccountID const& uHighAccountID,
                beast::Journal j);

/** Delete an offer.

    Requirements:
        The passed `sle` be obtained from a prior
        call to view.peek()
*/
TER
offerDelete (ApplyView& view,
    std::shared_ptr<SLE> const& sle,
        beast::Journal j);

//------------------------------------------------------------------------------

//
// Money Transfers
//

// Direct send w/o fees:
// - Redeeming IOUs and/or sending sender's own IOUs.
// - Create trust line of needed.
// --> bCheckIssuer : normally require issuer to be involved.
TER
mtchainCredit (ApplyView& view,
    AccountID const& uSenderID, AccountID const& uReceiverID,
    const STAmount & saAmount, bool bCheckIssuer,
    beast::Journal j);

TER
accountSend (ApplyView& view,
    AccountID const& from,
        AccountID const& to,
            const STAmount & saAmount,
                 beast::Journal j);

TER
issueIOU (ApplyView& view,
    AccountID const& account,
        STAmount const& amount,
            Issue const& issue,
                beast::Journal j);

TER
redeemIOU (ApplyView& view,
    AccountID const& account,
        STAmount const& amount,
            Issue const& issue,
                beast::Journal j);

TER
transferM (ApplyView& view,
    AccountID const& from,
        AccountID const& to,
            STAmount const& amount,
                beast::Journal j);

NetClock::time_point const& amendmentRIPD1141SoTime ();
bool amendmentRIPD1141 (NetClock::time_point const closeTime);

NetClock::time_point const& amendmentRIPD1274SoTime ();
bool amendmentRIPD1274 (NetClock::time_point const closeTime);

NetClock::time_point const& amendmentRIPD1298SoTime ();
bool amendmentRIPD1298 (NetClock::time_point const closeTime);

NetClock::time_point const& amendmentRIPD1443SoTime ();
bool amendmentRIPD1443 (NetClock::time_point const closeTime);

NetClock::time_point const& fix1449SoTime ();
bool fix1449 (NetClock::time_point const closeTime);

STAmount getIssueTotalAmount (Issue const& issue, ReadView const& view);
TER updateIssueTotalAmount (STAmount amount, ApplyView& view, beast::Journal j);
STAmount getIssueTotalAmount (Issue const& issue, ApplyView & view, beast::Journal j);
boost::optional<STAmount> getIssueLimitAmount(Issue const& issue, ReadView const& view);

std::pair<TER, bool>
addIssueInfo (STAmount const& amount, ApplyView& view, std::shared_ptr<SLE> &sleIssue,
              std::shared_ptr<SLE> const& sleOwner, beast::Journal j);

} //

#endif
