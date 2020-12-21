//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/beast/insight/Base.h>
#include <mtchain/beast/insight/BaseImpl.h>
#include <mtchain/beast/insight/CounterImpl.h>
#include <mtchain/beast/insight/EventImpl.h>
#include <mtchain/beast/insight/GaugeImpl.h>
#include <mtchain/beast/insight/MeterImpl.h>

namespace beast {
namespace insight {

Base::~Base ()
{
}

BaseImpl::~BaseImpl ()
{
}

CounterImpl::~CounterImpl ()
{
}

EventImpl::~EventImpl ()
{
}

GaugeImpl::~GaugeImpl ()
{
}

MeterImpl::~MeterImpl ()
{
}

}
}
