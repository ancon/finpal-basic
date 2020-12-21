//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <test/jtx/memo.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/protocol/JsonFields.h>

namespace mtchain {
namespace test {
namespace jtx {

void
memo::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoData"] = strHex(data_);
    m["MemoFormat"] = strHex(format_);
    m["MemoType"] = strHex(type_);
}

void
memodata::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoData"] = strHex(s_);
}

void
memoformat::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoFormat"] = strHex(s_);
}

void
memotype::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoType"] = strHex(s_);
}

void
memondata::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoFormat"] = strHex(format_);
    m["MemoType"] = strHex(type_);
}

void
memonformat::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoData"] = strHex(data_);
    m["MemoType"] = strHex(type_);
}

void
memontype::operator()(Env&, JTx& jt) const
{
    auto& jv = jt.jv;
    auto& ma = jv["Memos"];
    auto& mi = ma[ma.size()];
    auto& m = mi["Memo"];
    m["MemoData"] = strHex(data_);
    m["MemoFormat"] = strHex(format_);
}


} // jtx
} // test
} //
