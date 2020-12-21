//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_CHECKLIBRARYVERSIONS_H_INCLUDED
#define MTCHAIN_BASICS_CHECKLIBRARYVERSIONS_H_INCLUDED

#include <string>

namespace mtchain {
namespace version {

/** Check all library versions against MTChain's version requirements.

    Throws std::runtime_error if one or more libraries are out-of-date and do
    not meet the version requirements.
 */
void checkLibraryVersions();

}  // namespace version
}  // namespace mtchain

#endif
