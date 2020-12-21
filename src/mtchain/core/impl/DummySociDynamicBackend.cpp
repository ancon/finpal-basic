//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

/*  Stub functions for soci dynamic backends.

    MTChain does not use dynamic backends, and inclduing soci's
    dynamic backends compilcates the build (it requires a generated
    header file and some macros to be defined.)
*/

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/core/SociDB.h>
#include <soci/sqlite3/soci-sqlite3.h>

// dummy soci-backend
namespace soci {
namespace dynamic_backends {
// used internally by session
backend_factory const& get (std::string const& name)
{
    mtchain::Throw<std::runtime_error> ("Not Supported");
    return std::ref(soci::sqlite3); // Silence compiler warning.
};

// provided for advanced user-level management
std::vector<std::string>& search_paths ()
{
    static std::vector<std::string> empty;
    return empty;
};
void register_backend (std::string const&, std::string const&){};
void register_backend (std::string const&, backend_factory const&){};
std::vector<std::string> list_all ()
{
    return {};
};
void unload (std::string const&){};
void unload_all (){};

}  // namespace dynamic_backends
}  // namespace soci

