//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/Node.h>
#include <mtchain/app/paths/PathState.h>
#include <mtchain/protocol/JsonFields.h>

namespace mtchain {
namespace path {

// Compare the non-calculated fields.
bool Node::operator== (const Node& other) const
{
    return other.uFlags == uFlags
            && other.account_ == account_
            && other.issue_ == issue_;
}

// This is for debugging not end users. Output names can be changed without
// warning.
Json::Value Node::getJson () const
{
    Json::Value jvNode (Json::objectValue);
    Json::Value jvFlags (Json::arrayValue);

    jvNode[jss::type]  = uFlags;

    bool const hasCurrency = !isM (issue_.currency);
    bool const hasAccount = !isM (account_);
    bool const hasIssuer = !isM (issue_.account);

    if (isAccount() || hasAccount)
        jvFlags.append (!isAccount() == hasAccount ? "account" : "-account");

    if (uFlags & STPathElement::typeCurrency || hasCurrency)
    {
        jvFlags.append ((uFlags & STPathElement::typeCurrency) && hasCurrency
            ? "currency"
            : "-currency");
    }

    if (uFlags & STPathElement::typeIssuer || hasIssuer)
    {
        jvFlags.append ((uFlags & STPathElement::typeIssuer) && hasIssuer
            ? "issuer"
            : "-issuer");
    }

    jvNode["flags"] = jvFlags;

    if (!isM (account_))
        jvNode[jss::account] = to_string (account_);

    if (!isM (issue_.currency))
        jvNode[jss::currency] = to_string (issue_.currency);

    if (!isM (issue_.account))
        jvNode[jss::issuer] = to_string (issue_.account);

    if (saRevRedeem)
        jvNode["rev_redeem"] = saRevRedeem.getFullText ();

    if (saRevIssue)
        jvNode["rev_issue"] = saRevIssue.getFullText ();

    if (saRevDeliver)
        jvNode["rev_deliver"] = saRevDeliver.getFullText ();

    if (saFwdRedeem)
        jvNode["fwd_redeem"] = saFwdRedeem.getFullText ();

    if (saFwdIssue)
        jvNode["fwd_issue"] = saFwdIssue.getFullText ();

    if (saFwdDeliver)
        jvNode["fwd_deliver"] = saFwdDeliver.getFullText ();

    return jvNode;
}

} // path
} //
