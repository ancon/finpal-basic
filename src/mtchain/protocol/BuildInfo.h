//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_BUILDINFO_H_INCLUDED
#define MTCHAIN_PROTOCOL_BUILDINFO_H_INCLUDED

#include <cstdint>
#include <string>

namespace mtchain {

/** Describes a MTChain/RTXP protocol version. */
using ProtocolVersion = std::pair<std::uint16_t, std::uint16_t>;

/** Versioning information for this build. */
// VFALCO The namespace is deprecated
namespace BuildInfo {

/** Server version.
    Follows the Semantic Versioning Specification:
    http://semver.org/
*/
std::string const&
getVersionString();

/** Full server version string.
    This includes the name of the server. It is used in the peer
    protocol hello message and also the headers of some HTTP replies.
*/
std::string const&
getFullVersionString();

/** Construct a protocol version from a packed 32-bit protocol identifier */
ProtocolVersion
make_protocol (std::uint32_t version);

/** The protocol version we speak and prefer. */
ProtocolVersion const&
getCurrentProtocol();

/** The oldest protocol version we will accept. */
ProtocolVersion const& getMinimumProtocol ();

} // BuildInfo (DEPRECATED)

std::string
to_string (ProtocolVersion const& p);

std::uint32_t
to_packed (ProtocolVersion const& p);

} //

#endif
