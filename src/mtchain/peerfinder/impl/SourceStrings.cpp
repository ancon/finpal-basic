//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/peerfinder/impl/SourceStrings.h>

namespace mtchain {
namespace PeerFinder {

class SourceStringsImp : public SourceStrings
{
public:
    SourceStringsImp (std::string const& name, Strings const& strings)
        : m_name (name)
        , m_strings (strings)
    {
    }

    ~SourceStringsImp ()
    {
    }

    std::string const& name ()
    {
        return m_name;
    }

    void fetch (Results& results, beast::Journal journal)
    {
        results.addresses.resize (0);
        results.addresses.reserve (m_strings.size());
        for (int i = 0; i < m_strings.size (); ++i)
        {
            beast::IP::Endpoint ep (beast::IP::Endpoint::from_string (m_strings [i]));
            if (is_unspecified (ep))
                ep = beast::IP::Endpoint::from_string_altform (m_strings [i]);
            if (! is_unspecified (ep))
                results.addresses.push_back (ep);
        }
    }

private:
    std::string m_name;
    Strings m_strings;
};

//------------------------------------------------------------------------------

beast::SharedPtr <Source>
SourceStrings::New (std::string const& name, Strings const& strings)
{
    return new SourceStringsImp (name, strings);
}

}
}
