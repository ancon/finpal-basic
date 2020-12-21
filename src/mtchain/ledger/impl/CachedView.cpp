//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/ledger/CachedView.h>
#include <mtchain/basics/contract.h>
#include <mtchain/protocol/Serializer.h>

namespace mtchain {
namespace detail {

bool
CachedViewImpl::exists (Keylet const& k) const
{
    return read(k) != nullptr;
}

std::shared_ptr<SLE const>
CachedViewImpl::read (Keylet const& k) const
{
    {
        std::lock_guard<
            std::mutex> lock(mutex_);
        auto const iter = map_.find(k.key);
        if (iter != map_.end())
        {
            if (! k.check(*iter->second))
                return nullptr;
            return iter->second;
        }
    }
    auto const digest =
        base_.digest(k.key);
    if (! digest)
        return nullptr;
    auto sle = cache_.fetch(*digest,
        [&]() { return base_.read(k); });
    if (! sle)
        return nullptr;

    std::lock_guard<
        std::mutex> lock(mutex_);
    auto const iter =
        map_.find(k.key);
    if (iter == map_.end())
    {
        map_.emplace(k.key, sle);
        return sle;
    }
    if (! k.check(*iter->second))
        LogicError("CachedView::read: wrong type");
    return iter->second;
}

} // detail
} //
