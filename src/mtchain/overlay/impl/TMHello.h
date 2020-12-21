//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_OVERLAY_TMHELLO_H_INCLUDED
#define MTCHAIN_OVERLAY_TMHELLO_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/protocol/BuildInfo.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/UintTypes.h>
#include <beast/http/message.hpp>
#include <mtchain/beast/utility/Journal.h>
#include <utility>
#include <boost/asio/ssl.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_ref.hpp>
#include "mtchain.pb.h"

namespace mtchain {

enum
{
    // The clock drift we allow a remote peer to have
    clockToleranceDeltaSeconds = 20
};

/** Computes a shared value based on the SSL connection state.
    When there is no man in the middle, both sides will compute the same
    value. In the presence of an attacker, the computed values will be
    different.
    If the shared value generation fails, the link MUST be dropped.
    @return A pair. Second will be false if shared value generation failed.
*/
boost::optional<uint256>
makeSharedValue (SSL* ssl, beast::Journal journal);

/** Build a TMHello protocol message. */
protocol::TMHello
buildHello (uint256 const& sharedValue,
    beast::IP::Address public_ip,
    beast::IP::Endpoint remote, Application& app);

/** Insert HTTP headers based on the TMHello protocol message. */
void
appendHello (beast::http::fields& h, protocol::TMHello const& hello);

/** Parse HTTP headers into TMHello protocol message.
    @return A protocol message on success; an empty optional
            if the parsing failed.
*/
boost::optional<protocol::TMHello>
parseHello (bool request, beast::http::fields const& h, beast::Journal journal);

/** Validate and store the public key in the TMHello.
    This includes signature verification on the shared value.
    @return The remote end public key on success; an empty
            optional if the check failed.
*/
boost::optional<PublicKey>
verifyHello (protocol::TMHello const& h, uint256 const& sharedValue,
    beast::IP::Address public_ip,
    beast::IP::Endpoint remote,
    beast::Journal journal, Application& app);

/** Parse a set of protocol versions.
    The returned list contains no duplicates and is sorted ascending.
    Any strings that are not parseable as RTXP protocol strings are
    excluded from the result set.
*/
std::vector<ProtocolVersion>
parse_ProtocolVersions(boost::string_ref const& s);

}

#endif
