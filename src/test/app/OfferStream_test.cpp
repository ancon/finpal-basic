//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/OfferStream.h>
#include <mtchain/beast/unit_test.h>

namespace mtchain {

class OfferStream_test : public beast::unit_test::suite
{
public:
    void
    test()
    {
        pass();
    }

    void
    run()
    {
        test();
    }
};

BEAST_DEFINE_TESTSUITE(OfferStream,tx,mtchain);

}
