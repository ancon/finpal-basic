//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/basics/contract.h>
#include <mtchain/beast/unit_test.h>
#include <string>

namespace mtchain {

class contract_test : public beast::unit_test::suite
{
public:
    void run ()
    {
        try
        {
            Throw<std::runtime_error>("Throw test");
        }
        catch (std::runtime_error const& e)
        {
            BEAST_EXPECT(std::string(e.what()) == "Throw test");

            try
            {
                Rethrow();
            }
            catch (std::runtime_error const& e)
            {
                BEAST_EXPECT(std::string(e.what()) == "Throw test");
            }
            catch (...)
            {
                BEAST_EXPECT(false);
            }
        }
        catch (...)
        {
            BEAST_EXPECT(false);
        }
    }
};

BEAST_DEFINE_TESTSUITE(contract,basics,mtchain);

}
