//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RESOURCE_ENTRY_H_INCLUDED
#define MTCHAIN_RESOURCE_ENTRY_H_INCLUDED

#include <mtchain/basics/DecayingSample.h>
#include <mtchain/resource/impl/Key.h>
#include <mtchain/resource/impl/Tuning.h>
#include <mtchain/beast/clock/abstract_clock.h>
#include <mtchain/beast/core/List.h>
#include <cassert>

namespace mtchain {
namespace Resource {

using clock_type = beast::abstract_clock <std::chrono::steady_clock>;

// An entry in the table
// VFALCO DEPRECATED using boost::intrusive list
struct Entry
    : public beast::List <Entry>::Node
{
    Entry () = delete;

    /**
       @param now Construction time of Entry.
    */
    explicit Entry(clock_type::time_point const now)
        : refcount (0)
        , local_balance (now)
        , remote_balance (0)
        , lastWarningTime (0)
        , whenExpires (0)
    {
    }

    std::string to_string() const
    {
        switch (key->kind)
        {
        case kindInbound:   return key->address.to_string();
        case kindOutbound:  return key->address.to_string();
        case kindUnlimited: return std::string ("\"") + key->name + "\"";
        default:
            assert(false);
        }

        return "(undefined)";
    }

    /**
     * Returns `true` if this connection should have no
     * resource limits applied--it is still possible for certain RPC commands
     * to be forbidden, but that depends on Role.
     */
    bool isUnlimited () const
    {
        return key->kind == kindUnlimited;
    }

    // Balance including remote contributions
    int balance (clock_type::time_point const now)
    {
        return local_balance.value (now) + remote_balance;
    }

    // Add a charge and return normalized balance
    // including contributions from imports.
    int add (int charge, clock_type::time_point const now)
    {
        return local_balance.add (charge, now) + remote_balance;
    }

    // Back pointer to the map key (bit of a hack here)
    Key const* key;

    // Number of Consumer references
    int refcount;

    // Exponentially decaying balance of resource consumption
    DecayingSample <decayWindowSeconds, clock_type> local_balance;

    // Normalized balance contribution from imports
    int remote_balance;

    // Time of the last warning
    clock_type::rep lastWarningTime;

    // For inactive entries, time after which this entry will be erased
    clock_type::rep whenExpires;
};

inline std::ostream& operator<< (std::ostream& os, Entry const& v)
{
    os << v.to_string();
    return os;
}

}
}

#endif
