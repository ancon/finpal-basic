//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/InnerObjectFormats.h>

namespace mtchain {

InnerObjectFormats::InnerObjectFormats ()
{
    add (sfSignerEntry.getJsonName ().c_str (), sfSignerEntry.getCode ())
        << SOElement (sfAccount,              SOE_REQUIRED)
        << SOElement (sfSignerWeight,         SOE_REQUIRED)
        ;

    add (sfSigner.getJsonName ().c_str (), sfSigner.getCode ())
        << SOElement (sfAccount,              SOE_REQUIRED)
        << SOElement (sfSigningPubKey,        SOE_REQUIRED)
        << SOElement (sfTxnSignature,         SOE_REQUIRED)
        ;
}

void InnerObjectFormats::addCommonFields (Item& item)
{
}

InnerObjectFormats const&
InnerObjectFormats::getInstance ()
{
    static InnerObjectFormats instance;
    return instance;
}

SOTemplate const*
InnerObjectFormats::findSOTemplateBySField (SField const& sField) const
{
    SOTemplate const* ret = nullptr;
    auto itemPtr = findByType (sField.getCode ());
    if (itemPtr)
        ret = &(itemPtr->elements);

    return ret;
}

} //
