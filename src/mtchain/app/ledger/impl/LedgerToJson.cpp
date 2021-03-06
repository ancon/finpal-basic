//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/app/ledger/LedgerToJson.h>
#include <mtchain/basics/base_uint.h>

namespace mtchain {

namespace {

bool isFull(LedgerFill const& fill)
{
    return fill.options & LedgerFill::full;
}

bool isExpanded(LedgerFill const& fill)
{
    return isFull(fill) || (fill.options & LedgerFill::expand);
}

bool isBinary(LedgerFill const& fill)
{
    return fill.options & LedgerFill::binary;
}

template <class Object>
void fillJson(Object& json, bool closed, LedgerInfo const& info, bool bFull)
{
    json[jss::parent_hash]  = to_string (info.parentHash);
    json[jss::ledger_index] = to_string (info.seq);
    json[jss::seqNum]       = to_string (info.seq);      // DEPRECATED

    if (closed)
    {
        json[jss::closed] = true;
    }
    else if (!bFull)
    {
        json[jss::closed] = false;
        return;
    }

    json[jss::ledger_hash] = to_string (info.hash);
    json[jss::transaction_hash] = to_string (info.txHash);
    json[jss::account_hash] = to_string (info.accountHash);
    json[jss::total_coins] = to_string (info.drops);
	json[jss::fee_pool] = to_string(info.feePool);

    // These next three are DEPRECATED.
    json[jss::hash] = to_string (info.hash);
    json[jss::totalCoins] = to_string (info.drops);
    json[jss::accepted] = closed;
    json[jss::close_flags] = info.closeFlags;

    // Always show fields that contribute to the ledger hash
    json[jss::parent_close_time] = info.parentCloseTime.time_since_epoch().count();
    json[jss::close_time] = info.closeTime.time_since_epoch().count();
    json[jss::close_time_resolution] = info.closeTimeResolution.count();

    if (info.closeTime != NetClock::time_point{})
    {
        json[jss::close_time_human] = to_string(info.closeTime);
        if (! getCloseAgree(info))
            json[jss::close_time_estimated] = true;
    }
}

template <class Object>
void fillJsonBinary(Object& json, bool closed, LedgerInfo const& info)
{
    if (! closed)
        json[jss::closed] = false;
    else
    {
        json[jss::closed] = true;

        Serializer s;
        addRaw (info, s);
        json[jss::ledger_data] = strHex (s.peekData ());
    }
}

template <class Object>
void fillJsonTx (Object& json, LedgerFill const& fill)
{
    auto&& txns = setArray (json, jss::transactions);
    auto bBinary = isBinary(fill);
    auto bExpanded = isExpanded(fill);

    try
    {
        for (auto& i: fill.ledger.txs)
        {
            if (! bExpanded)
            {
                txns.append(to_string(i.first->getTransactionID()));
            }
            else
            {
                auto&& txJson = appendObject(txns);
                if (bBinary)
                {
                    txJson[jss::tx_blob] = serializeHex(*i.first);
                    if (i.second)
                        txJson[jss::meta] = serializeHex(*i.second);
                }
                else
                {
                    copyFrom(txJson, i.first->getJson(0));
                    if (i.second)
                        txJson[jss::metaData] = i.second->getJson(0);
                }

                if ((fill.options & LedgerFill::ownerFunds) &&
                    i.first->getTxnType() == ttOFFER_CREATE)
                {
                    auto const account = i.first->getAccountID(sfAccount);
                    auto const amount = i.first->getFieldAmount(sfTakerGets);

                    // If the offer create is not self funded then add the
                    // owner balance
                    if (account != amount.getIssuer())
                    {
                        auto const ownerFunds = accountFunds(fill.ledger,
                            account, amount, fhIGNORE_FREEZE, beast::Journal());
                        txJson[jss::owner_funds] = ownerFunds.getText ();
                    }
                }
            }
        }
    }
    catch (std::exception const&)
    {
        // Nothing the user can do about this.
    }
}

template <class Object>
void fillJsonState(Object& json, LedgerFill const& fill)
{
    auto& ledger = fill.ledger;
    auto&& array = Json::setArray (json, jss::accountState);
    auto expanded = isExpanded(fill);
    auto binary = isBinary(fill);

    for(auto const& sle : ledger.sles)
    {
        if (binary)
        {
            auto&& obj = appendObject(array);
            obj[jss::hash] = to_string(sle->key());
            obj[jss::tx_blob] = serializeHex(*sle);
        }
        else if (expanded)
            array.append(sle->getJson(0));
        else
            array.append(to_string(sle->key()));
    }
}

template <class Object>
void fillJson (Object& json, LedgerFill const& fill)
{
    // TODO: what happens if bBinary and bExtracted are both set?
    // Is there a way to report this back?
    auto bFull = isFull(fill);
    if (isBinary(fill))
        fillJsonBinary(json, !fill.ledger.open(), fill.ledger.info());
    else
        fillJson(json, !fill.ledger.open(), fill.ledger.info(), bFull);

    if (bFull || fill.options & LedgerFill::dumpTm)
        fillJsonTx(json, fill);

    if (bFull || fill.options & LedgerFill::dumpState)
        fillJsonState(json, fill);
}

} // namespace

void addJson (Json::Object& json, LedgerFill const& fill)
{
    auto&& object = Json::addObject (json, jss::ledger);
    fillJson (object, fill);
}

void addJson (Json::Value& json, LedgerFill const& fill)
{
    auto&& object = Json::addObject (json, jss::ledger);
    fillJson (object, fill);

}

Json::Value getJson (LedgerFill const& fill)
{
    Json::Value json;
    fillJson (json, fill);
    return json;
}

} //
