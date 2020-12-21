//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_PATHREQUEST_H_INCLUDED
#define MTCHAIN_APP_PATHS_PATHREQUEST_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/paths/Pathfinder.h>
#include <mtchain/app/paths/MTChainLineCache.h>
#include <mtchain/json/json_value.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/protocol/types.h>
#include <boost/optional.hpp>
#include <map>
#include <mutex>
#include <set>
#include <utility>

namespace mtchain {

// A pathfinding request submitted by a client
// The request issuer must maintain a strong pointer

class MTChainLineCache;
class PathRequests;

// Return values from parseJson <0 = invalid, >0 = valid
#define PFR_PJ_INVALID              -1
#define PFR_PJ_NOCHANGE             0
#define PFR_PJ_CHANGE               1

class PathRequest :
    public std::enable_shared_from_this <PathRequest>,
    public CountedObject <PathRequest>
{
public:
    static char const* getCountedObjectName () { return "PathRequest"; }

    using wptr    = std::weak_ptr<PathRequest>;
    using pointer = std::shared_ptr<PathRequest>;
    using ref     = const pointer&;
    using wref    = const wptr&;

public:
    // VFALCO TODO Break the cyclic dependency on InfoSub

    // path_find semantics
    // Subscriber is updated
    PathRequest (
        Application& app,
        std::shared_ptr <InfoSub> const& subscriber,
        int id,
        PathRequests&,
        beast::Journal journal);

    //_path_find semantics
    // Completion function is called
    PathRequest (
        Application& app,
        std::function <void (void)> const& completion,
        Resource::Consumer& consumer,
        int id,
        PathRequests&,
        beast::Journal journal);

    ~PathRequest ();

    bool isNew ();
    bool needsUpdate (bool newOnly, LedgerIndex index);
    void updateComplete ();

    std::pair<bool, Json::Value> doCreate (
        std::shared_ptr<MTChainLineCache> const&,
        Json::Value const&);

    Json::Value doClose (Json::Value const&);
    Json::Value doStatus (Json::Value const&);

    // update jvStatus
    Json::Value doUpdate (
        std::shared_ptr<MTChainLineCache> const&, bool fast);
    InfoSub::pointer getSubscriber ();
    bool hasCompletion ();

private:
    using ScopedLockType = std::lock_guard <std::recursive_mutex>;

    bool isValid (std::shared_ptr<MTChainLineCache> const& crCache);
    void setValid ();

    std::unique_ptr<Pathfinder> const&
    getPathFinder(std::shared_ptr<MTChainLineCache> const&,
        hash_map<Currency, std::unique_ptr<Pathfinder>>&, Currency const&,
            STAmount const&, int const);

    /** Finds and sets a PathSet in the JSON argument.
        Returns false if the source currencies are inavlid.
    */
    bool
    findPaths (std::shared_ptr<MTChainLineCache> const&, int const, Json::Value&);

    int parseJson (Json::Value const&);

    Application& app_;
    beast::Journal m_journal;

    std::recursive_mutex mLock;

    PathRequests& mOwner;

    std::weak_ptr<InfoSub> wpSubscriber; // Who this request came from
    std::function <void (void)> fCompletion;
    Resource::Consumer& consumer_; // Charge according to source currencies

    Json::Value jvId;
    Json::Value jvStatus; // Last result

    // Client request parameters
    boost::optional<AccountID> raSrcAccount;
    boost::optional<AccountID> raDstAccount;
    STAmount saDstAmount;
    boost::optional<STAmount> saSendMax;

    std::set<Issue> sciSourceCurrencies;
    std::map<Issue, STPathSet> mContext;

    bool convert_all_;

    std::recursive_mutex mIndexLock;
    LedgerIndex mLastIndex;
    bool mInProgress;

    int iLevel;
    bool bLastSuccess;

    int iIdentifier;

    std::chrono::steady_clock::time_point const created_;
    std::chrono::steady_clock::time_point quick_reply_;
    std::chrono::steady_clock::time_point full_reply_;

    static unsigned int const max_paths_ = 4;
};

} //

#endif
