//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/Log.h>
#include <mtchain/basics/RangeSet.h>
#include <mtchain/beast/core/LexicalCast.h>
#include <boost/foreach.hpp>
#include <cassert>
#include <cstdint>

namespace mtchain {

// VFALCO NOTE std::min and std::max not good enough?
//        NOTE Why isn't this written as a template?
//        TODO Replace this with std calls.
//
inline std::uint32_t min (std::uint32_t x, std::uint32_t y)
{
    return (x < y) ? x : y;
}
inline std::uint32_t max (std::uint32_t x, std::uint32_t y)
{
    return (x > y) ? x : y;
}

bool RangeSet::hasValue (std::uint32_t v) const
{
    for (auto const& it : mRanges)
    {
        if (contains (it, v))
            return true;
    }
    return false;
}

std::uint32_t RangeSet::getFirst () const
{
    const_iterator it = mRanges.begin ();

    if (it == mRanges.end ())
        return absent;

    return it->first;
}

std::uint32_t RangeSet::getNext (std::uint32_t v) const
{
    for (auto const& it : mRanges)
    {
        if (it.first > v)
            return it.first;

        if (contains (it, v + 1))
            return v + 1;
    }
    return absent;
}

std::uint32_t RangeSet::getLast () const
{
    const_reverse_iterator it = mRanges.rbegin ();

    if (it == mRanges.rend ())
        return absent;

    return it->second;
}

std::uint32_t RangeSet::getPrev (std::uint32_t v) const
{
    BOOST_REVERSE_FOREACH (const value_type & it, mRanges)
    {
        if (it.second < v)
            return it.second;

        if (contains (it, v - 1)) // TODO: Here may be error
            return v - 1;
    }
    return absent;
}

RangeSet* createEmptyRangeSet() { return new RangeSet(); }
  
// Return the largest number not in the set that is less than the given number
//
std::uint32_t RangeSet::prevMissing (std::uint32_t v) const
{
    std::uint32_t result = absent;

    if (v != 0)
    {
        checkInternalConsistency ();

        // Handle the case where the loop reaches the terminating condition
        //
        result = v - 1;

        for (const_reverse_iterator cur = mRanges.rbegin (); cur != mRanges.rend (); ++cur)
        {
            // See if v-1 is in the range
            if (contains (*cur, result))
            {
                result = cur->first - 1;
                break;
            }

	    if (result > cur->second)
	    {
                break;
	    }
        }
    }

    assert (result == absent || !hasValue (result));

    return result;
}

//RangeSet is used to represent all 'full' ledger ranges now
void RangeSet::setValue (std::uint32_t v)
{ // add a value into a range or become an independent range
    if (!hasValue (v))
    {
        mRanges[v] = v;

        simplify ();
    }
}

void RangeSet::setRange (std::uint32_t minV, std::uint32_t maxV)
{
    while (hasValue (minV))
    {
        ++minV;

        if (minV >= maxV)
            return;
    }

    mRanges[minV] = maxV;

    simplify ();
}

void RangeSet::clearValue (std::uint32_t v)
{
    for (iterator it = mRanges.begin (); it != mRanges.end (); ++it)
    {
        if (contains (*it, v))
        {
            if (it->first == v)
            {
                if (it->second == v)
                {
                    mRanges.erase (it);
                }
                else
                {
                    std::uint32_t oldEnd = it->second;
                    mRanges.erase(it);
                    mRanges[v + 1] = oldEnd;
                }
            }
            else if (it->second == v)
            {
                -- (it->second);
            }
            else
            {
                std::uint32_t oldEnd = it->second;
                it->second = v - 1;
                mRanges[v + 1] = oldEnd;
            }

            checkInternalConsistency();
            return;
        }
    }
}

std::string RangeSet::toString () const
{
    std::string ret;
    for (auto const& it : mRanges)
    {
        if (!ret.empty ())
            ret += ",";

        if (it.first == it.second)
            ret += beast::lexicalCastThrow <std::string> ((it.first));
        else
            ret += beast::lexicalCastThrow <std::string> (it.first) + "-"
                   + beast::lexicalCastThrow <std::string> (it.second);
    }

    if (ret.empty ())
        return "empty";

    return ret;
}

void RangeSet::simplify () // combine continual ranges into a range
{
    iterator it = mRanges.begin ();

    while (1)
    {
        iterator nit = it;

        if (++nit == mRanges.end ())
        {
            checkInternalConsistency();
            return;
        }

        if (it->second >= (nit->first - 1))
        {
            // ranges overlap
            it->second = std::max(it->second, nit->second);
            mRanges.erase (nit);
        }
        else
        {
            it = nit;
        }
    }
}

std::size_t
RangeSet::lebesgue_sum() const
{
    std::size_t sum = mRanges.size();
    for (auto const& e : mRanges)
        sum += e.second - e.first;
    return sum;
}

void RangeSet::checkInternalConsistency () const noexcept
{
#ifndef NDEBUG
    if (mRanges.size () > 1)
    {
        const_iterator const last = std::prev (mRanges.end ());

        for (const_iterator cur = mRanges.begin (); cur != last; ++cur)
        {
            const_iterator const next = std::next (cur);
            assert (cur->first <= cur->second);
            assert (next->first <= next->second);
            assert (cur->second + 1 < next->first);
        }
    }
    else if (mRanges.size () == 1)
    {
        const_iterator const iter = mRanges.begin ();
        assert (iter->first <= iter->second);
    }
#endif
}

} //
