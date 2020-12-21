//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_FLOW_H_INCLUDED
#define MTCHAIN_APP_PATHS_FLOW_H_INCLUDED

#include <mtchain/app/paths/impl/Steps.h>
#include <mtchain/app/paths/MtchainCalc.h>
#include <mtchain/protocol/Quality.h>

namespace mtchain
{

namespace path {
namespace detail{
struct FlowDebugInfo;
}
}

/**
  Make a payment from the src account to the dst account

  @param view Trust lines and balances
  @param deliver Amount to deliver to the dst account
  @param src Account providing input funds for the payment
  @param dst Account receiving the payment
  @param paths Set of paths to explore for liquidity
  @param defaultPaths Include defaultPaths in the path set
  @param partialPayment If the payment cannot deliver the entire
           requested amount, deliver as much as possible, given the constraints
  @param limitQuality Do not use liquidity below this quality threshold
  @param sendMax Do not spend more than this amount
  @param logs Logs to write journal messages to
  @return Actual amount in and out, and the result code
*/
path::MtchainCalc::Output
flow (PaymentSandbox& view,
    STAmount const& deliver,
    AccountID const& src,
    AccountID const& dst,
    STPathSet const& paths,
    bool defaultPaths,
    bool partialPayment,
    bool ownerPaysTransferFee,
    boost::optional<Quality> const& limitQuality,
    boost::optional<STAmount> const& sendMax,
    beast::Journal j,
    path::detail::FlowDebugInfo* flowDebugInfo=nullptr);

}  //

#endif
