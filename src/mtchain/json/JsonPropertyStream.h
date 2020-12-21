//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_JSON_JSONPROPERTYSTREAM_H_INCLUDED
#define MTCHAIN_JSON_JSONPROPERTYSTREAM_H_INCLUDED

#include <mtchain/json/json_value.h>
#include <mtchain/beast/utility/PropertyStream.h>

namespace mtchain {

/** A PropertyStream::Sink which produces a Json::Value of type objectValue. */
class JsonPropertyStream : public beast::PropertyStream
{
public:
    Json::Value m_top;
    std::vector <Json::Value*> m_stack;

public:
    JsonPropertyStream ();
    Json::Value const& top() const;

protected:

    void map_begin ();
    void map_begin (std::string const& key);
    void map_end ();
    void add (std::string const& key, short value);
    void add (std::string const& key, unsigned short value);
    void add (std::string const& key, int value);
    void add (std::string const& key, unsigned int value);
    void add (std::string const& key, long value);
    void add (std::string const& key, float v);
    void add (std::string const& key, double v);
    void add (std::string const& key, std::string const& v);
    void array_begin ();
    void array_begin (std::string const& key);
    void array_end ();

    void add (short value);
    void add (unsigned short value);
    void add (int value);
    void add (unsigned int value);
    void add (long value);
    void add (float v);
    void add (double v);
    void add (std::string const& v);
};

}

#endif
