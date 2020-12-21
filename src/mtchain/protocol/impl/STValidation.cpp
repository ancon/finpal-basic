//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/STValidation.h>
#include <mtchain/protocol/HashPrefix.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>
#include <mtchain/json/to_string.h>

namespace mtchain {

STValidation::STValidation (SerialIter& sit, bool checkSignature)
    : STObject (getFormat (), sit, sfValidation)
{
    mNodeID = calcNodeID(
        PublicKey(makeSlice (getFieldVL (sfSigningPubKey))));
    assert (mNodeID.isNonZero ());

    if  (checkSignature && !isValid ())
    {
        JLOG (debugLog().error())
            << "Invalid validation" << getJson (0);
        Throw<std::runtime_error> ("Invalid validation");
    }
}

STValidation::STValidation (
        uint256 const& ledgerHash,
        NetClock::time_point signTime,
        PublicKey const& publicKey,
        bool isFull)
    : STObject (getFormat (), sfValidation)
    , mSeen (signTime)
{
    // Does not sign
    setFieldH256 (sfLedgerHash, ledgerHash);
    setFieldU32 (sfSigningTime, signTime.time_since_epoch().count());

    setFieldVL (sfSigningPubKey, publicKey.slice());
    mNodeID = calcNodeID(publicKey);
    assert (mNodeID.isNonZero ());

    if (isFull)
        setFlag (kFullFlag);
}

uint256 STValidation::sign (SecretKey const& secretKey)
{
    setFlag (vfFullyCanonicalSig);

    auto const signingHash = getSigningHash();
    setFieldVL (sfSignature,
        signDigest (getSignerPublic(), secretKey, signingHash));
    return signingHash;
}

uint256 STValidation::getSigningHash () const
{
    return STObject::getSigningHash (HashPrefix::validation);
}

uint256 STValidation::getLedgerHash () const
{
    return getFieldH256 (sfLedgerHash);
}

NetClock::time_point
STValidation::getSignTime () const
{
    return NetClock::time_point{NetClock::duration{getFieldU32(sfSigningTime)}};
}

NetClock::time_point STValidation::getSeenTime () const
{
    return mSeen;
}

std::uint32_t STValidation::getFlags () const
{
    return getFieldU32 (sfFlags);
}

bool STValidation::isValid () const
{
    return isValid (getSigningHash ());
}

bool STValidation::isValid (uint256 const& signingHash) const
{
    try
    {
        return verifyDigest (getSignerPublic(),
            signingHash,
            makeSlice(getFieldVL (sfSignature)),
            getFlags () & vfFullyCanonicalSig);
    }
    catch (std::exception const&)
    {
        JLOG (debugLog().error())
            << "Exception validating validation";
        return false;
    }
}

PublicKey STValidation::getSignerPublic () const
{
    return PublicKey(makeSlice (getFieldVL (sfSigningPubKey)));
}

bool STValidation::isFull () const
{
    return (getFlags () & kFullFlag) != 0;
}

Blob STValidation::getSignature () const
{
    return getFieldVL (sfSignature);
}

Blob STValidation::getSigned () const
{
    Serializer s;
    add (s);
    return s.peekData ();
}

SOTemplate const& STValidation::getFormat ()
{
    struct FormatHolder
    {
        SOTemplate format;

        FormatHolder ()
        {
            format.push_back (SOElement (sfFlags,           SOE_REQUIRED));
            format.push_back (SOElement (sfLedgerHash,      SOE_REQUIRED));
            format.push_back (SOElement (sfLedgerSequence,  SOE_OPTIONAL));
            format.push_back (SOElement (sfCloseTime,       SOE_OPTIONAL));
            format.push_back (SOElement (sfLoadFee,         SOE_OPTIONAL));
            format.push_back (SOElement (sfAmendments,      SOE_OPTIONAL));
            format.push_back (SOElement (sfBaseFee,         SOE_OPTIONAL));
            format.push_back (SOElement (sfReserveBase,     SOE_OPTIONAL));
            format.push_back (SOElement (sfReserveIncrement, SOE_OPTIONAL));
            format.push_back (SOElement (sfSigningTime,     SOE_REQUIRED));
            format.push_back (SOElement (sfSigningPubKey,   SOE_REQUIRED));
            format.push_back (SOElement (sfSignature,       SOE_OPTIONAL));
        }
    };

    static FormatHolder holder;

    return holder.format;
}

} //
