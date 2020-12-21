//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/crypto/KeyType.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/protocol/Seed.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/handlers/WalletPropose.h>
#include <ed25519-donna/ed25519.h>
#include <boost/optional.hpp>
#include <cmath>
#include <map>

namespace mtchain {

double
estimate_entropy (std::string const& input)
{
    // First, we calculate the Shannon entropy. This gives
    // the average number of bits per symbol that we would
    // need to encode the input.
    std::map<int, double> freq;

    for (auto const& c : input)
        freq[c]++;

    double se = 0.0;

    for (auto const& f : freq)
    {
        auto x = f.second / input.length();
        se += (x) * log2(x);
    }

    // We multiply it by the length, to get an estimate of
    // the number of bits in the input. We floor because it
    // is better to be conservative.
    return std::floor (-se * input.length());
}

// {
//  passphrase: <string>
// }
Json::Value doWalletPropose (RPC::Context& context)
{
    return walletPropose (context.params);
}

Json::Value walletPropose (Json::Value const& params)
{
    bool has_key_type = params.isMember (jss::key_type);
    boost::optional<Seed> seed;
    KeyType keyType = KeyType::secp256k1;

    if (has_key_type)
    {
        if (! params[jss::key_type].isString())
        {
            return RPC::expected_field_error (
                jss::key_type, "string");
        }

        keyType = keyTypeFromString (
            params[jss::key_type].asString());

        if (keyType == KeyType::invalid)
            return RPC::make_error(rpcINVALID_PARAMS, "WalletPropose: invalid key type");
    }

    if (params.isMember (jss::passphrase) ||
        params.isMember (jss::seed) ||
        params.isMember (jss::seed_hex))
    {
        Json::Value err;
        seed = RPC::getSeedFromRPC (params, err);
        if (!seed)
            return err;

	if (seed->type() != KeyType::unknown)
        {
            if (has_key_type)
            {
                if (keyType != seed->type())
                {
                    return RPC::make_error(rpcBAD_SEED, "WalletPropose: "
                                           "seed type doesn't match key type");
                }
            }
            else
            {
                keyType = seed->type();
            }
        }
        else
        {
            seed->setType (keyType);
        }
    }
    else
    {
        seed = randomSeed ();
        seed->setType (keyType);
    }

    if (!isSupportKeyType(keyType))
    {
        return RPC::make_error(rpcINVALID_PARAMS, "WalletPropose: invalid key type");
    }

    auto const keypair = generateKeyPair (keyType, *seed);
    auto const publicKey = keypair.first;
    auto const privateKey = keypair.second;

    Json::Value obj (Json::objectValue);

    obj[jss::master_seed] = toBase58 (*seed);
    obj[jss::master_seed_hex] = strHex (seed->data(), seed->size());
    obj[jss::master_key] = seedAs1751 (*seed);
    obj[jss::account_id] = toBase58(calcAccountID(publicKey));
    obj[jss::public_key] = toBase58(TOKEN_ACCOUNT_PUBLIC, publicKey);
    obj[jss::key_type] = to_string (keyType);
    obj[jss::public_key_hex] = strHex (publicKey.data(), publicKey.size());
    //obj[jss::private_key_hex] = strHex (privateKey.data(), privateKey.size());

    if (params.isMember (jss::passphrase))
    {
        auto const entropy = estimate_entropy (
            params[jss::passphrase].asString());

        // 80 bits of entropy isn't bad, but it's better to
        // err on the side of caution and be conservative.
        if (entropy < 80.0)
            obj[jss::warning] =
                "This wallet was generated using a user-supplied "
                "passphrase that has low entropy and is vulnerable "
                "to brute-force attacks.";
        else
            obj[jss::warning] =
                "This wallet was generated using a user-supplied "
                "passphrase. It may be vulnerable to brute-force "
                "attacks.";
    }

    return obj;
}

} //
