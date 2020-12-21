//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/shamap/SHAMapMissingNode.h>
#include <ostream>

namespace mtchain {

std::ostream&
operator<< (std::ostream& out, const SHAMapMissingNode& mn)
{
    switch (mn.mType)
    {
    case SHAMapType::TRANSACTION:
        out << "Missing/TXN(";
        break;

    case SHAMapType::STATE:
        out << "Missing/STA(";
        break;

    case SHAMapType::FREE:
    default:
        out << "Missing/(";
        break;
    };

    if (mn.mNodeHash == zero)
        out << "id : " << mn.mNodeID;
    else
        out << "hash : " << mn.mNodeHash;
    out << ")";
    return out;
}

} //
