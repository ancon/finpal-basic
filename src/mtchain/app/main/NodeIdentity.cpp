//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/core/DatabaseCon.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/main/NodeIdentity.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/core/ConfigSections.h>
#include <boost/format.hpp>
#include <boost/optional.hpp>

namespace mtchain {

std::pair<PublicKey, SecretKey>
loadNodeIdentity (Application& app)
{
    // If a seed is specified in the configuration file use that directly.
    if (app.config().exists(SECTION_NODE_SEED))
    {
        auto const seed = parseBase58<Seed>(
            app.config().section(SECTION_NODE_SEED).lines().front());

        if (!seed)
            Throw<std::runtime_error>(
                "NodeIdentity: Bad [" SECTION_NODE_SEED "] specified");

        auto secretKey =
            generateSecretKey (KeyType::secp256k1, *seed);
        auto publicKey =
            derivePublicKey (KeyType::secp256k1, secretKey);

        return { publicKey, secretKey };
    }

    // Try to load a node identity from the database:
    boost::optional<PublicKey> publicKey;
    boost::optional<SecretKey> secretKey;

    auto db = app.getWalletDB ().checkoutDb ();

    {
        boost::optional<std::string> pubKO, priKO;
        soci::statement st = (db->prepare <<
            "SELECT PublicKey, PrivateKey FROM NodeIdentity;",
                soci::into(pubKO),
                soci::into(priKO));
        st.execute ();
        while (st.fetch ())
        {
            auto const sk = parseBase58<SecretKey>(
                TOKEN_NODE_PRIVATE, priKO.value_or(""));
            auto const pk = parseBase58<PublicKey>(
                TOKEN_NODE_PUBLIC, pubKO.value_or(""));

            // Only use if the public and secret keys are a pair
            if (sk && pk && (*pk == derivePublicKey (KeyType::secp256k1, *sk)))
            {
                secretKey = sk;
                publicKey = pk;
            }
        }
    }

    // If a valid identity wasn't found, we randomly generate a new one:
    if (!publicKey || !secretKey)
    {
        std::tie(publicKey, secretKey) = randomKeyPair(KeyType::secp256k1);

        *db << str (boost::format (
            "INSERT INTO NodeIdentity (PublicKey,PrivateKey) VALUES ('%s','%s');")
                % toBase58 (TokenType::TOKEN_NODE_PUBLIC, *publicKey)
                % toBase58 (TokenType::TOKEN_NODE_PRIVATE, *secretKey));
    }

    return { *publicKey, *secretKey };
}

} //
