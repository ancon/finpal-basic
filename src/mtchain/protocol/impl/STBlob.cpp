//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/STBlob.h>
#include <mtchain/basics/StringUtilities.h>

namespace mtchain {

STBlob::STBlob (SerialIter& st, SField const& name)
    : STBase (name)
    , value_ (st.getVLBuffer ())
{
}

std::string
STBlob::getText () const
{
    return strHex (value_.data (), value_.size ());
}

bool
STBlob::isEquivalent (const STBase& t) const
{
    const STBlob* v = dynamic_cast<const STBlob*> (&t);
    return v && (value_ == v->value_);
}

} //
