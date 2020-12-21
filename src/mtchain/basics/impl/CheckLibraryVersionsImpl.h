//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_CHECKLIBRARYVERSIONSIMPL_H_INCLUDED
#define MTCHAIN_BASICS_CHECKLIBRARYVERSIONSIMPL_H_INCLUDED

#include <mtchain/basics/CheckLibraryVersions.h>

namespace mtchain {
namespace version {

/** Both Boost and OpenSSL have integral version numbers. */
using VersionNumber = unsigned long long;

std::string
boostVersion(VersionNumber boostVersion);

std::string
openSSLVersion(VersionNumber openSSLVersion);

void checkVersion(
    std::string name,
    std::string required,
    std::string actual);

void checkBoost(std::string version);
void checkOpenSSL(std::string version);

}  // namespace version
}  // namespace mtchain

#endif
