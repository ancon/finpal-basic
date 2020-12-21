//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_STPARSEDJSON_H_INCLUDED
#define MTCHAIN_PROTOCOL_STPARSEDJSON_H_INCLUDED

#include <mtchain/protocol/STArray.h>
#include <boost/optional.hpp>

namespace mtchain {

/** Holds the serialized result of parsing an input JSON object.
    This does validation and checking on the provided JSON.
*/
class STParsedJSONObject
{
public:
    /** Parses and creates an STParsedJSON object.
        The result of the parsing is stored in object and error.
        Exceptions:
            Does not throw.
        @param name The name of the JSON field, used in diagnostics.
        @param json The JSON-RPC to parse.
    */
    STParsedJSONObject (std::string const& name, Json::Value const& json);

    STParsedJSONObject () = delete;
    STParsedJSONObject (STParsedJSONObject const&) = delete;
    STParsedJSONObject& operator= (STParsedJSONObject const&) = delete;
    ~STParsedJSONObject () = default;

    /** The STObject if the parse was successful. */
    boost::optional <STObject> object;

    /** On failure, an appropriate set of error values. */
    Json::Value error;
};

/** Holds the serialized result of parsing an input JSON array.
    This does validation and checking on the provided JSON.
*/
class STParsedJSONArray
{
public:
    /** Parses and creates an STParsedJSON array.
        The result of the parsing is stored in array and error.
        Exceptions:
            Does not throw.
        @param name The name of the JSON field, used in diagnostics.
        @param json The JSON-RPC to parse.
    */
    STParsedJSONArray (std::string const& name, Json::Value const& json);

    STParsedJSONArray () = delete;
    STParsedJSONArray (STParsedJSONArray const&) = delete;
    STParsedJSONArray& operator= (STParsedJSONArray const&) = delete;
    ~STParsedJSONArray () = default;

    /** The STArray if the parse was successful. */
    boost::optional <STArray> array;

    /** On failure, an appropriate set of error values. */
    Json::Value error;
};



} //

#endif
