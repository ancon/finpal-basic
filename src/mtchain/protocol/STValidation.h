//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_STVALIDATION_H_INCLUDED
#define MTCHAIN_PROTOCOL_STVALIDATION_H_INCLUDED

#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/STObject.h>
#include <cstdint>
#include <memory>

namespace mtchain {

// Validation flags
const std::uint32_t vfFullyCanonicalSig    = 0x80000000; // signature is fully canonical

class STValidation final
    : public STObject
    , public CountedObject <STValidation>
{
public:
    static char const* getCountedObjectName () { return "STValidation"; }

    using pointer = std::shared_ptr<STValidation>;
    using ref     = const std::shared_ptr<STValidation>&;

    enum
    {
        kFullFlag = 0x1
    };

    // These throw if the object is not valid
    STValidation (SerialIter & sit, bool checkSignature = true);

    // Does not sign the validation
    STValidation (
        uint256 const& ledgerHash,
        NetClock::time_point signTime,
        PublicKey const& raPub,
        bool isFull);

    STBase*
    copy (std::size_t n, void* buf) const override
    {
        return emplace(n, buf, *this);
    }

    STBase*
    move (std::size_t n, void* buf) override
    {
        return emplace(n, buf, std::move(*this));
    }

    uint256         getLedgerHash ()     const;
    NetClock::time_point getSignTime ()  const;
    NetClock::time_point getSeenTime ()  const;
    std::uint32_t   getFlags ()          const;
    PublicKey       getSignerPublic ()   const;
    NodeID          getNodeID ()         const
    {
        return mNodeID;
    }
    bool            isValid ()           const;
    bool            isFull ()            const;
    bool            isTrusted ()         const
    {
        return mTrusted;
    }
    uint256         getSigningHash ()    const;
    bool            isValid (uint256 const& ) const;

    void            setTrusted ()
    {
        mTrusted = true;
    }
    void            setSeen (NetClock::time_point s)
    {
        mSeen = s;
    }
    Blob    getSigned ()                 const;
    Blob    getSignature ()              const;

    // Signs the validation and returns the signing hash
    uint256 sign (SecretKey const& secretKey);

    // The validation this replaced
    uint256 const& getPreviousHash ()
    {
        return mPreviousHash;
    }
    bool isPreviousHash (uint256 const& h) const
    {
        return mPreviousHash == h;
    }
    void setPreviousHash (uint256 const& h)
    {
        mPreviousHash = h;
    }

private:
    static SOTemplate const& getFormat ();

    void setNode ();

    uint256 mPreviousHash;
    NodeID mNodeID;
    bool mTrusted = false;
    NetClock::time_point mSeen = {};
};

} //

#endif
