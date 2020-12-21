//------------------------------------------------------------------------------
/*
This file is part of FinPald: https://github.com/finpal/finpal-basic
Copyright (c) 2019 ~ 2020 FinPal Alliance.

Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================
#include <mtchain/beast/unit_test.h>
#include <mtchain/beast/utility/Debug.h>
namespace beast {

// A simple unit test to determine the diagnostic settings in a build.
//
class Debug_test : public unit_test::suite
{
public:
    static int envDebug()
    {
#ifdef _DEBUG
        return 1;
#else
        return 0;
#endif
    }

    static int beastDebug()
    {
#ifdef BEAST_DEBUG
        return BEAST_DEBUG;
#else
        return 0;
#endif
    }

    static int beastForceDebug()
    {
#ifdef BEAST_FORCE_DEBUG
        return BEAST_FORCE_DEBUG;
#else
        return 0;
#endif
    }

    void run()
    {
        log <<
            "_DEBUG              = " << envDebug() << '\n' <<
            "BEAST_DEBUG         = " << beastDebug() << '\n' <<
            "BEAST_FORCE_DEBUG   = " << beastForceDebug() << '\n' <<
            "sizeof(std::size_t) = " << sizeof(std::size_t) << std::endl;
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(Debug, utility, beast);

}