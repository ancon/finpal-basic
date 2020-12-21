//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2014, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/beast/unit_test.h>
#include <mtchain/nodestore/impl/varint.h>
#include <array>
#include <vector>

namespace mtchain {
namespace NodeStore {
namespace tests {

class varint_test : public beast::unit_test::suite
{
public:
    void
    test_varints (std::vector<std::size_t> vv)
    {
        testcase("encode, decode");
        for (auto const v : vv)
        {
            std::array<std::uint8_t,
                varint_traits<
                    std::size_t>::max> vi;
            auto const n0 =
                write_varint(
                    vi.data(), v);
            expect (n0 > 0, "write error");
            std::size_t v1;
            auto const n1 =
                read_varint(
                    vi.data(), n0, v1);
            expect(n1 == n0, "read error");
            expect(v == v1, "wrong value");
        }
    }

    void
    run() override
    {
        test_varints({
                0,     1,     2,
              126,   127,   128,
              253,   254,   255,
            16127, 16128, 16129,
            0xff,
            0xffff,
            0xffffffff,
            0xffffffffffffUL,
            0xffffffffffffffffUL});
    }
};

BEAST_DEFINE_TESTSUITE(varint,NodeStore,mtchain);

} // test
} // NodeStore
} //
