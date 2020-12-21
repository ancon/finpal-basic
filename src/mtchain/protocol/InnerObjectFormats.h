//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_INNER_OBJECT_FORMATS_H_INCLUDED
#define MTCHAIN_PROTOCOL_INNER_OBJECT_FORMATS_H_INCLUDED

#include <mtchain/protocol/KnownFormats.h>

namespace mtchain {

/** Manages the list of known inner object formats.
*/
class InnerObjectFormats : public KnownFormats <int>
{
private:
    void addCommonFields (Item& item);

public:
    /** Create the object.
        This will load the object will all the known inner object formats.
    */
    InnerObjectFormats ();

    static InnerObjectFormats const& getInstance ();

    SOTemplate const* findSOTemplateBySField (SField const& sField) const;
};

} //

#endif
