//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/Output.h>
#include <mtchain/json/Writer.h>

namespace Json {

namespace {

void outputJson (Json::Value const& value, Writer& writer)
{
    switch (value.type())
    {
    case Json::nullValue:
    {
        writer.output (nullptr);
        break;
    }

    case Json::intValue:
    {
        writer.output (value.asInt());
        break;
    }

    case Json::uintValue:
    {
        writer.output (value.asUInt());
        break;
    }

    case Json::realValue:
    {
        writer.output (value.asDouble());
        break;
    }

    case Json::stringValue:
    {
        writer.output (value.asString());
        break;
    }

    case Json::booleanValue:
    {
        writer.output (value.asBool());
        break;
    }

    case Json::arrayValue:
    {
        writer.startRoot (Writer::array);
        for (auto const& i: value)
        {
            writer.rawAppend();
            outputJson (i, writer);
        }
        writer.finish();
        break;
    }

    case Json::objectValue:
    {
        writer.startRoot (Writer::object);
        auto members = value.getMemberNames ();
        for (auto const& tag: members)
        {
            writer.rawSet (tag);
            outputJson (value[tag], writer);
        }
        writer.finish();
        break;
    }
    } // switch
}

} // namespace

void outputJson (Json::Value const& value, Output const& out)
{
    Writer writer (out);
    outputJson (value, writer);
}

std::string jsonAsString (Json::Value const& value)
{
    std::string s;
    Writer writer (stringOutput (s));
    outputJson (value, writer);
    return s;
}

} // Json
