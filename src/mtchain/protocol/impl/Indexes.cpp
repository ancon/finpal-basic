//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/protocol/Indexes.h>
#include <cassert>

namespace mtchain {

// get the index of the node that holds the last 256 ledgers
uint256
getLedgerHashIndex ()
{
    return sha512Half(std::uint16_t(spaceSkipList));
}

// Get the index of the node that holds the set of 256 ledgers that includes
// this ledger's hash (or the first ledger after it if it's not a multiple
// of 256).
uint256
getLedgerHashIndex (std::uint32_t desiredLedgerIndex)
{
    return sha512Half(
        std::uint16_t(spaceSkipList),
        std::uint32_t(desiredLedgerIndex >> 16));
}
// get the index of the node that holds the enabled amendments
uint256
getLedgerAmendmentIndex ()
{
    return sha512Half(std::uint16_t(spaceAmendment));
}

// get the index of the node that holds the fee schedule
uint256
getLedgerFeeIndex ()
{
    return sha512Half(std::uint16_t(spaceFee));
}

uint256
getAccountRootIndex (AccountID const& account)
{
    return sha512Half(
        std::uint16_t(spaceAccount),
        account);
}

uint256
getGeneratorIndex (AccountID const& uGeneratorID)
{
    return sha512Half(
        std::uint16_t(spaceGenerator),
        uGeneratorID);
}

uint256
getBookBase (Book const& book)
{
    assert (isConsistent (book));
    // Return with quality 0.
    return getQualityIndex(sha512Half(
        std::uint16_t(spaceBookDir),
        book.in.currency,
        book.out.currency,
        book.in.account,
        book.out.account));
}

uint256
getOfferIndex (AccountID const& account, std::uint32_t uSequence)
{
    return sha512Half(
        std::uint16_t(spaceOffer),
        account,
        std::uint32_t(uSequence));
}

uint256
getOwnerDirIndex (AccountID const& account)
{
    return sha512Half(
        std::uint16_t(spaceOwnerDir),
        account);
}


uint256
getDirNodeIndex (uint256 const& uDirRoot, const std::uint64_t uNodeIndex)
{
    if (uNodeIndex == 0)
        return uDirRoot;

    return sha512Half(
        std::uint16_t(spaceDirNode),
        uDirRoot,
        std::uint64_t(uNodeIndex));
}

uint256
getQualityIndex (uint256 const& uBase, const std::uint64_t uNodeDir)
{
    // Indexes are stored in big endian format: they print as hex as stored.
    // Most significant bytes are first.  Least significant bytes represent
    // adjacent entries.  We place uNodeDir in the 8 right most bytes to be
    // adjacent.  Want uNodeDir in big endian format so ++ goes to the next
    // entry for indexes.
    uint256 uNode (uBase);

    // TODO(tom): there must be a better way.
    // VFALCO [base_uint] This assumes a certain storage format
    ((std::uint64_t*) uNode.end ())[-1] = htobe64 (uNodeDir);

    return uNode;
}

uint256
getQualityNext (uint256 const& uBase)
{
    static uint256 const uNext (
        from_hex_text<uint256>("10000000000000000"));
    return uBase + uNext;
}

std::uint64_t
getQuality (uint256 const& uBase)
{
    // VFALCO [base_uint] This assumes a certain storage format
    return be64toh (((std::uint64_t*) uBase.end ())[-1]);
}

uint256
getTicketIndex (AccountID const& account, std::uint32_t uSequence)
{
    return sha512Half(
        std::uint16_t(spaceTicket),
        account,
        std::uint32_t(uSequence));
}

uint256
getMtchainStateIndex (AccountID const& a, AccountID const& b, Currency const& currency)
{
    if (a < b)
        return sha512Half(
            std::uint16_t(spaceMTChain),
            a,
            b,
            currency);
    return sha512Half(
        std::uint16_t(spaceMTChain),
        b,
        a,
        currency);
}

uint256
getMtchainStateIndex (AccountID const& a, Issue const& issue)
{
    return getMtchainStateIndex (a, issue.account, issue.currency);
}

uint256
getSignerListIndex (AccountID const& account)
{
    // We are prepared for there to be multiple SignerLists in the future,
    // but we don't have them yet.  In anticipation of multiple SignerLists
    // We supply a 32-bit ID to locate the SignerList.  Until we actually
    // *have* multiple signer lists, we can default that ID to zero.
    return sha512Half(
        std::uint16_t(spaceSignerList),
        account,
        std::uint32_t (0));  // 0 == default SignerList ID.
}

static inline uint256 getNFTokenIndex (uint256 const& assetid, Blob const& id)
{
    return sha512Half(std::uint16_t(spaceNFToken),
                      assetid,
                      id);
}

static inline uint256 getNFTokenOwnerAuthIndex (uint256 const& assetid, AccountID const& owner,
                                                AccountID const& oper)
{
    return sha512Half(std::uint16_t(spaceNFTAuth),
                      assetid,
                      owner,
                      oper);
}

static inline uint256 getNFTokenAccountIndex(uint256 const& assetid, AccountID const& owner)
{
    return sha512Half(std::uint16_t(spaceNFTAccount),
                      assetid,
                      owner);
}

static inline uint256 getNFTokenIndexIndex(uint256 const& assetid, std::uint64_t tokenIndex)
{
    return sha512Half(std::uint16_t(spaceNFTIndex),
                      assetid,
                      tokenIndex);
}

static inline uint256 getNFTokenOwnerIndexIndex(uint256 const& assetid, AccountID const& owner,
                                                std::uint64_t ownerTokenIndex)
{
    return sha512Half(std::uint16_t(spaceNFTOwnerIndex),
                      assetid,
                      owner,
                      ownerTokenIndex);
}

static inline uint256 getNFAssetIndex(AccountID const& issuer, Blob const& id)
{
    return sha512Half(std::uint16_t(spaceNFAsset),
                      issuer,
                      id);
}

static inline uint256 getNFAssetIndexIndex(AccountID const& account, std::uint64_t assetIndex)
{
    return sha512Half(std::uint16_t(spaceNFAssetIndex),
                      account,
                      assetIndex);
}

//------------------------------------------------------------------------------

namespace keylet {

Keylet account_t::operator()(
    AccountID const& id) const
{
    return { ltACCOUNT_ROOT,
        getAccountRootIndex(id) };
}

Keylet child (uint256 const& key)
{
    return { ltCHILD, key };
}

Keylet skip_t::operator()() const
{
    return { ltLEDGER_HASHES,
        getLedgerHashIndex() };
}

Keylet skip_t::operator()(LedgerIndex ledger) const
{
    return { ltLEDGER_HASHES,
        getLedgerHashIndex(ledger) };
}

Keylet amendments_t::operator()() const
{
    return { ltAMENDMENTS,
        getLedgerAmendmentIndex() };
}

Keylet fees_t::operator()() const
{
    return { ltFEE_SETTINGS,
        getLedgerFeeIndex() };
}

Keylet book_t::operator()(Book const& b) const
{
    return { ltDIR_NODE,
        getBookBase(b) };
}

Keylet line_t::operator()(AccountID const& id0,
    AccountID const& id1, Currency const& currency) const
{
    return { ltMTCHAIN_STATE,
        getMtchainStateIndex(id0, id1, currency) };
}

Keylet line_t::operator()(AccountID const& id,
    Issue const& issue) const
{
    return { ltMTCHAIN_STATE,
        getMtchainStateIndex(id, issue) };
}

Keylet offer_t::operator()(AccountID const& id,
    std::uint32_t seq) const
{
    return { ltOFFER,
        getOfferIndex(id, seq) };
}

Keylet quality_t::operator()(Keylet const& k,
    std::uint64_t q) const
{
    assert(k.type == ltDIR_NODE);
    return { ltDIR_NODE,
        getQualityIndex(k.key, q) };
}

Keylet next_t::operator()(Keylet const& k) const
{
    assert(k.type == ltDIR_NODE);
    return { ltDIR_NODE,
        getQualityNext(k.key) };
}

Keylet ticket_t::operator()(AccountID const& id,
    std::uint32_t seq) const
{
    return { ltTICKET,
        getTicketIndex(id, seq) };
}

Keylet signers_t::operator()(AccountID const& id) const
{
    return { ltSIGNER_LIST,
        getSignerListIndex(id) };
}

Keylet nftoken_t::operator()(uint256 const& assetid, Blob const& id) const
{
    return { ltNFTOKEN, getNFTokenIndex(assetid, id) };
}

Keylet nftoken_t::operator()(uint256 const& assetid, std::uint64_t tokenIndex) const
{
    return { ltNFT_INDEX, getNFTokenIndexIndex(assetid, tokenIndex) };
}

Keylet nftoken_t::operator()(uint256 const& assetid, AccountID const& owner,
                             std::uint64_t ownerTokenIndex) const
{
    return { ltNFT_OWNER_INDEX, getNFTokenOwnerIndexIndex(assetid, owner, ownerTokenIndex) };
}

Keylet nfasset_t::operator()(AccountID const& issuer, Blob const& id) const
{
    return { ltNFASSET, getNFAssetIndex(issuer, id) };
}

Keylet nfasset_t::operator()(uint256 const& assetid, AccountID const& owner) const
{
    return { ltNFT_ACCOUNT, getNFTokenAccountIndex(assetid, owner) };
}

Keylet nfasset_t::operator()(uint256 const& assetid, AccountID const& owner,
                             AccountID const& oper) const
{
    return { ltNFTAUTH, getNFTokenOwnerAuthIndex(assetid, owner, oper) };
}

Keylet nfasset_t::operator()(AccountID const& account, std::uint64_t assetIndex) const
{
    return { ltNFASSET_INDEX, getNFAssetIndexIndex(account, assetIndex) };
}


//------------------------------------------------------------------------------

Keylet unchecked (uint256 const& key)
{
    return { ltANY, key };
}

Keylet ownerDir(AccountID const& id)
{
    return { ltDIR_NODE,
        getOwnerDirIndex(id) };
}

Keylet page(uint256 const& key,
    std::uint64_t index)
{
    return { ltDIR_NODE,
        getDirNodeIndex(key, index) };
}

Keylet page(Keylet const& root,
    std::uint64_t index)
{
    assert(root.type == ltDIR_NODE);
    return page(root.key, index);
}

Keylet
escrow (AccountID const& source, std::uint32_t seq)
{
    sha512_half_hasher h;
    using beast::hash_append;
    hash_append(h, std::uint16_t(spaceEscrow));
    hash_append(h, source);
    hash_append(h, seq);
    return { ltESCROW, static_cast<uint256>(h) };
}

Keylet
payChan (AccountID const& source, AccountID const& dst, std::uint32_t seq)
{
    sha512_half_hasher h;
    using beast::hash_append;
    hash_append(h, std::uint16_t(spaceMUChannel));
    hash_append(h, source);
    hash_append(h, dst);
    hash_append(h, seq);
    return { ltPAYCHAN, static_cast<uint256>(h) };
}

} // keylet

} //
