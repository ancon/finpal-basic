//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/Slice.h>
#include <mtchain/basics/strHex.h>
#include <algorithm>

namespace mtchain {

int charUnHex (unsigned char c)
{
    struct HexTab
    {
        int hex[256];

        HexTab ()
        {
            std::fill (std::begin (hex), std::end (hex), -1);
            for (int i = 0; i < 10; ++i)
                hex ['0'+i] = i;
            for (int i = 0; i < 6; ++i)
            {
                hex ['A'+i] = 10 + i;
                hex ['a'+i] = 10 + i;
            }
        }
        int operator[] (unsigned char c) const
        {
            return hex[c];
        }
    };

    static HexTab xtab;

    return xtab[c];
}

std::string
strHex(Slice const& slice)
{
    return strHex(slice.data(), slice.size());
}

}
