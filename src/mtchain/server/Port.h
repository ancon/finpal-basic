//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_PORT_H_INCLUDED
#define MTCHAIN_SERVER_PORT_H_INCLUDED

#include <mtchain/basics/BasicConfig.h>
#include <mtchain/beast/net/IPEndpoint.h>
#include <beast/core/detail/ci_char_traits.hpp>
#include <beast/websocket/option.hpp>
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <memory>
#include <set>
#include <string>

namespace boost { namespace asio { namespace ssl { class context; } } }

namespace mtchain {

/** Configuration information for a Server listening port. */
struct Port
{
    std::string name;
    boost::asio::ip::address ip;
    std::uint16_t port = 0;
    std::set<std::string, beast::detail::ci_less> protocol;
    std::vector<beast::IP::Address> admin_ip;
    std::vector<beast::IP::Address> secure_gateway_ip;
    std::string user;
    std::string password;
    std::string admin_user;
    std::string admin_password;
    std::string ssl_key;
    std::string ssl_cert;
    std::string ssl_chain;
    std::string ssl_ciphers;
    beast::websocket::permessage_deflate pmd_options;
    std::shared_ptr<boost::asio::ssl::context> context;

    // How many incoming connections are allowed on this
    // port in the range [0, 65535] where 0 means unlimited.
    int limit = 0;

    // Returns `true` if any websocket protocols are specified
    bool websockets() const;

    // Returns `true` if any secure protocols are specified
    bool secure() const;

    // Returns a string containing the list of protocols
    std::string protocols() const;
};

std::ostream&
operator<< (std::ostream& os, Port const& p);

//------------------------------------------------------------------------------

struct ParsedPort
{
    std::string name;
    std::set<std::string, beast::detail::ci_less> protocol;
    std::string user;
    std::string password;
    std::string admin_user;
    std::string admin_password;
    std::string ssl_key;
    std::string ssl_cert;
    std::string ssl_chain;
    std::string ssl_ciphers;
    beast::websocket::permessage_deflate pmd_options;
    int limit = 0;

    boost::optional<boost::asio::ip::address> ip;
    boost::optional<std::uint16_t> port;
    boost::optional<std::vector<beast::IP::Address>> admin_ip;
    boost::optional<std::vector<beast::IP::Address>> secure_gateway_ip;
};

void
parse_Port (ParsedPort& port, Section const& section, std::ostream& log);

} //

#endif
