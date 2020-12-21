//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_HANDLERS_LEDGER_H_INCLUDED
#define MTCHAIN_RPC_HANDLERS_LEDGER_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/app/ledger/LedgerToJson.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/json/Object.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/Status.h>
#include <mtchain/rpc/impl/Handler.h>
#include <mtchain/rpc/Role.h>

namespace Json {
class Object;
}

namespace mtchain {
namespace RPC {

struct Context;

// ledger [id|index|current|closed] [full]
// {
//    ledger: 'current' | 'closed' | <uint256> | <number>,  // optional
//    full: true | false    // optional, defaults to false.
// }

class LedgerHandler {
public:
    explicit LedgerHandler (Context&);

    Status check ();

    template <class Object>
    void writeResult (Object&);

    static const char* const name()
    {
        return "ledger";
    }

    static Role role()
    {
        return Role::USER;
    }

    static Condition condition()
    {
        return NO_CONDITION;
    }

private:
    Context& context_;
    std::shared_ptr<ReadView const> ledger_;
    Json::Value result_;
    int options_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Implementation.

template <class Object>
void LedgerHandler::writeResult (Object& value)
{
    if (ledger_)
    {
        Json::copyFrom (value, result_);
        addJson (value, {*ledger_, options_});
    }
    else
    {
        auto& master = context_.app.getLedgerMaster ();
        {
            auto&& closed = Json::addObject (value, jss::closed);
            addJson (closed, {*master.getClosedLedger(), 0});
        }
        {
            auto&& open = Json::addObject (value, jss::open);
            addJson (open, {*master.getCurrentLedger(), 0});
        }
    }
}

} // RPC
} //

#endif
