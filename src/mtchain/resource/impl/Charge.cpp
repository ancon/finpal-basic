//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/resource/Charge.h>
#include <sstream>

namespace mtchain {
namespace Resource {

Charge::Charge (value_type cost, std::string const& label)
    : m_cost (cost)
    , m_label (label)
{
}

std::string const& Charge::label () const
{
    return m_label;
}

Charge::value_type Charge::cost() const
{
    return m_cost;
}

std::string Charge::to_string () const
{
    std::stringstream ss;
    ss << m_label << " ($" << m_cost << ")";
    return ss.str();
}

std::ostream& operator<< (std::ostream& os, Charge const& v)
{
    os << v.to_string();
    return os;
}

bool Charge::operator== (Charge const& c) const
{
    return c.m_cost == m_cost;
}

bool Charge::operator!= (Charge const& c) const
{
    return c.m_cost != m_cost;
}

}
}
