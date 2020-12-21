//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_LEDGER_DIR_H_INCLUDED
#define MTCHAIN_LEDGER_DIR_H_INCLUDED

#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

class Dir
{
private:
    ReadView const* view_ = nullptr;
    Keylet root_;
    std::shared_ptr<SLE const> sle_;
    STVector256 const* indexes_ = nullptr;

public:
    class const_iterator;
    using value_type = std::shared_ptr<SLE const>;

    Dir(ReadView const&, Keylet const&);

    const_iterator
    begin() const;

    const_iterator
    end() const;

    const_iterator
    find(uint256 const& page_key, uint256 const& sle_key) const;
};

class Dir::const_iterator
{
public:
    using value_type =
        Dir::value_type;
    using pointer =
        value_type const*;
    using reference =
        value_type const&;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    const_iterator() = default;

    bool
    operator==(const_iterator const& other) const;

    bool
    operator!=(const_iterator const& other) const
    {
        return ! (*this == other);
    }

    reference
    operator*() const;

    pointer
    operator->() const
    {
        return &**this;
    }

    const_iterator&
    operator++();

    const_iterator
    operator++(int);

    Keylet const&
    page() const
    {
        return page_;
    }

    uint256
    index() const
    {
        return index_;
    }

private:
    friend class Dir;

    const_iterator(ReadView const& view,
        Keylet const& root, Keylet const& page)
    : view_(&view)
    , root_(root)
    , page_(page)
    {
    }

    ReadView const* view_ = nullptr;
    Keylet root_;
    Keylet page_;
    uint256 index_;
    boost::optional<value_type> mutable cache_;
    std::shared_ptr<SLE const> sle_;
    STVector256 const* indexes_ = nullptr;
    std::vector<uint256>::const_iterator it_;
};

} //

#endif
