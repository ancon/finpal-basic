//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef BEAST_INSIGHT_HOOKIMPL_H_INCLUDED
#define BEAST_INSIGHT_HOOKIMPL_H_INCLUDED

#include <mtchain/beast/insight/BaseImpl.h>

namespace beast {
namespace insight {

class HookImpl
    : public std::enable_shared_from_this <HookImpl>
    , public BaseImpl
{
public:
    using HandlerType = std::function <void (void)>;

    virtual ~HookImpl () = 0;
};

}
}

#endif
