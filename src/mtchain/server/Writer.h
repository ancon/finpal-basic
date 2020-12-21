//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_SERVER_WRITER_H_INCLUDED
#define MTCHAIN_SERVER_WRITER_H_INCLUDED

#include <boost/asio/buffer.hpp>
#include <functional>
#include <vector>

namespace mtchain {

class Writer
{
public:
    virtual ~Writer() = default;

    /** Returns `true` if there is no more data to pull. */
    virtual
    bool
    complete() = 0;

    /** Removes bytes from the input sequence.

        Can be called with 0.
    */
    virtual
    void
    consume (std::size_t bytes) = 0;

    /** Add data to the input sequence.
        @param bytes A hint to the number of bytes desired.
        @param resume A functor to later resume execution.
        @return `true` if the writer is ready to provide more data.
    */
    virtual
    bool
    prepare (std::size_t bytes,
        std::function<void(void)> resume) = 0;

    /** Returns a ConstBufferSequence representing the input sequence. */
    virtual
    std::vector<boost::asio::const_buffer>
    data() = 0;
};

} //

#endif
