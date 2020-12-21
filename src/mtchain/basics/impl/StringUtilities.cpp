//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Slice.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/basics/ToString.h>
#include <mtchain/beast/core/LexicalCast.h>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/regex.hpp>
#include <algorithm>
#include <cstdarg>

namespace mtchain {

std::pair<Blob, bool> strUnHex (std::string const& strSrc)
{
    Blob out;

    out.reserve ((strSrc.size () + 1) / 2);

    auto iter = strSrc.cbegin ();

    if (strSrc.size () & 1)
    {
        int c = charUnHex (*iter);

        if (c < 0)
            return std::make_pair (Blob (), false);

        out.push_back(c);
        ++iter;
    }

    while (iter != strSrc.cend ())
    {
        int cHigh = charUnHex (*iter);
        ++iter;

        if (cHigh < 0)
            return std::make_pair (Blob (), false);

        int cLow = charUnHex (*iter);
        ++iter;

        if (cLow < 0)
            return std::make_pair (Blob (), false);

        out.push_back (static_cast<unsigned char>((cHigh << 4) | cLow));
    }

    return std::make_pair(std::move(out), true);
}

uint64_t uintFromHex (std::string const& strSrc)
{
    uint64_t uValue (0);

    if (strSrc.size () > 16)
        Throw<std::invalid_argument> ("overlong 64-bit value");

    for (auto c : strSrc)
    {
        int ret = charUnHex (c);

        if (ret == -1)
            Throw<std::invalid_argument> ("invalid hex digit");

        uValue = (uValue << 4) | ret;
    }

    return uValue;
}

// TODO Callers should be using beast::URL and beast::parse_URL instead.
bool parseUrl (parsedURL& pUrl, std::string const& strUrl)
{
    // scheme://username:password@hostname:port/rest
    static boost::regex reUrl ("(?i)\\`\\s*([[:alpha:]][-+.[:alpha:][:digit:]]*)://([^:/]+)(?::(\\d+))?(/.*)?\\s*?\\'");
    boost::smatch smMatch;

    bool bMatch = boost::regex_match (strUrl, smMatch, reUrl); // Match status code.

    if (bMatch)
    {
        std::string strPort;

        pUrl.scheme = smMatch[1];
        boost::algorithm::to_lower (pUrl.scheme);
        pUrl.domain = smMatch[2];
        if (smMatch[3].length ())
        {
            pUrl.port = beast::lexicalCast <std::uint16_t> (
                std::string (smMatch[3]));
        }
        pUrl.path = smMatch[4];
    }

    return bMatch;
}

std::string trim_whitespace (std::string str)
{
    boost::trim (str);
    return str;
}

bool isHexStr (std::string const& txid, std::uint32_t size)
{
    if (txid.size () != size)
        return false;

    auto const ret = std::find_if (txid.begin (), txid.end (),
        [](std::string::value_type c)
        {
            return !std::isxdigit (c);
        });

    return (ret == txid.end ());
}

std::string to_string(Blob const& b)
{
    std::string s;

    for (auto c : b) {
        s.push_back(c);
    }

    return s;
}

std::tuple<std::uint64_t, std::uint64_t> parse_fraction(std::string const& s)
{
    unsigned long num, den;
    auto pos = s.find('/');
    if (pos == std::string::npos)
    {
        num = stoul(s);
        den = 1;
    } else {
        num = stoul(s.substr(0, pos));
        den = stoul(s.substr(pos + 1));
    }

    return std::make_tuple(num, den);
}

Blob to_blob(const char* s)
{
    Blob b;

    if (s) {
        while (*s) {
            b.push_back (*s);
            ++s;
        }
    }

    return b;
}

} //
