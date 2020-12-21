//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef BEAST_INSIGHT_NULLCOLLECTOR_H_INCLUDED
#define BEAST_INSIGHT_NULLCOLLECTOR_H_INCLUDED

#include <mtchain/beast/insight/Collector.h>

namespace beast {
namespace insight {

/** A Collector which does not collect metrics. */
class NullCollector : public Collector
{
public:
    static std::shared_ptr <Collector> New ();
};

}
}

#endif
