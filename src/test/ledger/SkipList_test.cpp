//-----------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/basics/Log.h>
#include <mtchain/ledger/View.h>
#include <test/jtx.h>
#include <mtchain/beast/unit_test.h>

namespace mtchain {
namespace test {

class SkipList_test : public beast::unit_test::suite
{
    void
    testSkipList()
    {
        beast::Journal const j;
        std::vector<std::shared_ptr<Ledger>> history;
        {
            jtx::Env env(*this);
            Config config;
            auto prev = std::make_shared<Ledger>(
                create_genesis, config,
                std::vector<uint256>{}, env.app().family());
            history.push_back(prev);
            for (auto i = 0; i < 1023; ++i)
            {
                auto next = std::make_shared<Ledger>(
                    *prev,
                    env.app().timeKeeper().closeTime());
                next->updateSkipList();
                history.push_back(next);
                prev = next;
            }
        }

        {
            auto l = *(std::next(std::begin(history)));
            BEAST_EXPECT((*std::begin(history))->info().seq <
                l->info().seq);
            BEAST_EXPECT(hashOfSeq(*l, l->info().seq + 1,
                j) == boost::none);
            BEAST_EXPECT(hashOfSeq(*l, l->info().seq,
                j) == l->info().hash);
            BEAST_EXPECT(hashOfSeq(*l, l->info().seq - 1,
                j) == l->info().parentHash);
            BEAST_EXPECT(hashOfSeq(*history.back(),
                l->info().seq, j) == boost::none);
        }

        // ledger skip lists store up to the previous 256 hashes
        for (auto i = history.crbegin();
            i != history.crend(); i += 256)
        {
            for (auto n = i;
                n != std::next(i,
                    (*i)->info().seq - 256 > 1 ? 257 : 256);
                ++n)
            {
                BEAST_EXPECT(hashOfSeq(**i,
                    (*n)->info().seq, j) ==
                        (*n)->info().hash);
            }

            // edge case accessing beyond 256
            BEAST_EXPECT(hashOfSeq(**i,
                (*i)->info().seq - 258, j) ==
                    boost::none);
        }

        // every 256th hash beyond the first 256 is stored
        for (auto i = history.crbegin();
            i != std::next(history.crend(), -512);
            i += 256)
        {
            for (auto n = std::next(i, 512);
                n != history.crend();
                n += 256)
            {
                BEAST_EXPECT(hashOfSeq(**i,
                    (*n)->info().seq, j) ==
                        (*n)->info().hash);
            }
        }
    }

    void run()
    {
        testSkipList();
    }
};

BEAST_DEFINE_TESTSUITE(SkipList,ledger,mtchain);

}  // test
}  //
