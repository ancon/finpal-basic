//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_BASICS_MAKE_SSLCONTEXT_H_INCLUDED
#define MTCHAIN_BASICS_MAKE_SSLCONTEXT_H_INCLUDED

#include <boost/asio/ssl/context.hpp>
#include <string>

namespace mtchain {

/** Create a self-signed SSL context that allows anonymous Diffie Hellman. */
std::shared_ptr<boost::asio::ssl::context>
make_SSLContext(
    std::string cipherList);

/** Create an authenticated SSL context using the specified files. */
std::shared_ptr<boost::asio::ssl::context>
make_SSLContextAuthed (
    std::string keyFile,
    std::string certFile,
    std::string chainFile,
    std::string cipherList);


}

#endif
