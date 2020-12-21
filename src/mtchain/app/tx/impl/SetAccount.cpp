//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/SetAccount.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/protocol/Feature.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/st.h>
#include <mtchain/ledger/View.h>


namespace mtchain {

TER
delIssueInfo (STAmount const& amount, ApplyView& view, std::shared_ptr<SLE> const& sleOwner,
              std::function<bool (SLE::ref)> f, beast::Journal j);
  
bool
SetAccount::affectsSubsequentTransactionAuth(STTx const& tx)
{
    auto const uTxFlags = tx.getFlags();
    if(uTxFlags & (tfRequireAuth | tfOptionalAuth))
        return true;

    auto const uSetFlag = tx[~sfSetFlag];
    if(uSetFlag && (*uSetFlag == asfRequireAuth ||
        *uSetFlag == asfDisableMaster ||
            *uSetFlag == asfAccountTxnID))
                return true;

    auto const uClearFlag = tx[~sfClearFlag];
    return uClearFlag && (*uClearFlag == asfRequireAuth ||
        *uClearFlag == asfDisableMaster ||
            *uClearFlag == asfAccountTxnID);
}

TER
SetAccount::preflight (PreflightContext const& ctx)
{
    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    auto& tx = ctx.tx;
    auto& j = ctx.j;

    std::uint32_t const uTxFlags = tx.getFlags ();

    if (uTxFlags & tfAccountSetMask)
    {
        JLOG(j.trace()) << "Malformed transaction: Invalid flags set.";
        return temINVALID_FLAG;
    }

    std::uint32_t const uSetFlag = tx.getFieldU32 (sfSetFlag);
    std::uint32_t const uClearFlag = tx.getFieldU32 (sfClearFlag);

    if ((uSetFlag != 0) && (uSetFlag == uClearFlag))
    {
        JLOG(j.trace()) << "Malformed transaction: Set and clear same flag.";
        return temINVALID_FLAG;
    }

    //
    // RequireAuth
    //
    bool bSetRequireAuth   = (uTxFlags & tfRequireAuth) || (uSetFlag == asfRequireAuth);
    bool bClearRequireAuth = (uTxFlags & tfOptionalAuth) || (uClearFlag == asfRequireAuth);

    if (bSetRequireAuth && bClearRequireAuth)
    {
        JLOG(j.trace()) << "Malformed transaction: Contradictory flags set.";
        return temINVALID_FLAG;
    }

    //
    // RequireDestTag
    //
    bool bSetRequireDest   = (uTxFlags & TxFlag::requireDestTag) || (uSetFlag == asfRequireDest);
    bool bClearRequireDest = (uTxFlags & tfOptionalDestTag) || (uClearFlag == asfRequireDest);

    if (bSetRequireDest && bClearRequireDest)
    {
        JLOG(j.trace()) << "Malformed transaction: Contradictory flags set.";
        return temINVALID_FLAG;
    }

    //
    // DisallowM
    //
    bool bSetDisallowM   = (uTxFlags & tfDisallowM) || (uSetFlag == asfDisallowM);
    bool bClearDisallowM = (uTxFlags & tfAllowM) || (uClearFlag == asfDisallowM);

    if (bSetDisallowM && bClearDisallowM)
    {
        JLOG(j.trace()) << "Malformed transaction: Contradictory flags set.";
        return temINVALID_FLAG;
    }

    // TransferRate
    if (tx.isFieldPresent (sfTransferRate))
    {
        std::uint32_t uRate = tx.getFieldU32 (sfTransferRate);

        if (uRate && (uRate < QUALITY_ONE))
        {
            JLOG(j.trace()) << "Malformed transaction: Bad transfer rate.";
            return temBAD_TRANSFER_RATE;
        }
    }

    // TickSize
    if (tx.isFieldPresent (sfTickSize))
    {
        if (!ctx.rules.enabled(featureTickSize))
            return temDISABLED;

        auto uTickSize = tx[sfTickSize];
        if (uTickSize &&
            ((uTickSize < Quality::minTickSize) ||
            (uTickSize > Quality::maxTickSize)))
        {
            JLOG(j.trace()) << "Malformed transaction: Bad tick size.";
            return temBAD_TICK_SIZE;
        }
    }

    if (auto const mk = tx[~sfMessageKey])
    {
        if (mk->size() && ! publicKeyType ({mk->data(), mk->size()}))
        {
            JLOG(j.trace()) << "Invalid message key specified.";
            return telBAD_PUBLIC_KEY;
        }
    }

    auto const domain = tx[~sfDomain];
    if (domain && domain->size() > DOMAIN_BYTES_MAX)
    {
        JLOG(j.trace()) << "domain too long";
        return telBAD_DOMAIN;
    }

    return preflight2(ctx);
}

TER
SetAccount::preclaim(PreclaimContext const& ctx)
{
    auto const id = ctx.tx[sfAccount];

    std::uint32_t const uTxFlags = ctx.tx.getFlags();

    auto const sle = ctx.view.read(
        keylet::account(id));

    std::uint32_t const uFlagsIn = sle->getFieldU32(sfFlags);

    std::uint32_t const uSetFlag = ctx.tx.getFieldU32(sfSetFlag);

    // legacy AccountSet flags
    bool bSetRequireAuth = (uTxFlags & tfRequireAuth) || (uSetFlag == asfRequireAuth);

    //
    // RequireAuth
    //
    if (bSetRequireAuth && !(uFlagsIn & lsfRequireAuth))
    {
        if (!dirIsEmpty(ctx.view,
            keylet::ownerDir(id)))
        {
            JLOG(ctx.j.trace()) << "Retry: Owner directory not empty.";
            return (ctx.flags & tapRETRY) ? terOWNERS : tecOWNERS;
        }
    }

    if (uSetFlag == asfIOUPrecision && !ctx.tx.isFieldPresent(sfAmount))
    {
        return temINVALID;
    }

    return tesSUCCESS;
}

/*static*/ bool canDeleteSLE(std::shared_ptr<SLE> const& sle)
{
    for (auto const& f : *sle)
    {
        if (f.getSType() == STI_NOTPRESENT)
            continue;

        auto const* pField = &f.getFName();

        if (&sfLedgerEntryType == pField)
            continue;

        if (&sfFlags == pField)
            continue;

        if (&sfBalance == pField)
        {
            auto const& balance = sle->getFieldAmount(sfBalance);
            if (balance.signum() != 0)
                return false;

            continue;
        }

        if (&sfLowLimit == pField)
            continue;

        if (&sfHighLimit == pField)
            continue;

        if (&sfPreviousTxnID == pField)
            continue;

        if (&sfPreviousTxnLgrSeq == pField)
            continue;

        if (&sfLowNode == pField)
            continue;

        if (&sfHighNode == pField)
            continue;

        return false;
    }

    return true;
}

TER
SetAccount::doApply ()
{
    std::uint32_t const uTxFlags = ctx_.tx.getFlags ();

    auto const sle = view().peek(
        keylet::account(account_));

    std::uint32_t const uFlagsIn = sle->getFieldU32 (sfFlags);
    std::uint32_t uFlagsOut = uFlagsIn;

    std::uint32_t const uSetFlag = ctx_.tx.getFieldU32 (sfSetFlag);
    std::uint32_t const uClearFlag = ctx_.tx.getFieldU32 (sfClearFlag);

    // legacy AccountSet flags
    bool bSetRequireDest   = (uTxFlags & TxFlag::requireDestTag) || (uSetFlag == asfRequireDest);
    bool bClearRequireDest = (uTxFlags & tfOptionalDestTag) || (uClearFlag == asfRequireDest);
    bool bSetRequireAuth   = (uTxFlags & tfRequireAuth) || (uSetFlag == asfRequireAuth);
    bool bClearRequireAuth = (uTxFlags & tfOptionalAuth) || (uClearFlag == asfRequireAuth);
    bool bSetDisallowM   = (uTxFlags & tfDisallowM) || (uSetFlag == asfDisallowM);
    bool bClearDisallowM = (uTxFlags & tfAllowM) || (uClearFlag == asfDisallowM);

    bool sigWithMaster = false;

    {
        auto const spk = ctx_.tx.getSigningPubKey();

        if (publicKeyType (makeSlice (spk)))
        {
            PublicKey const signingPubKey (makeSlice (spk));

            if (calcAccountID(signingPubKey) == account_)
                sigWithMaster = true;
        }
    }

    //
    // RequireAuth
    //
    if (bSetRequireAuth && !(uFlagsIn & lsfRequireAuth))
    {
        JLOG(j_.trace()) << "Set RequireAuth.";
        uFlagsOut |= lsfRequireAuth;
    }

    if (bClearRequireAuth && (uFlagsIn & lsfRequireAuth))
    {
        JLOG(j_.trace()) << "Clear RequireAuth.";
        uFlagsOut &= ~lsfRequireAuth;
    }

    //
    // RequireDestTag
    //
    if (bSetRequireDest && !(uFlagsIn & lsfRequireDestTag))
    {
        JLOG(j_.trace()) << "Set lsfRequireDestTag.";
        uFlagsOut |= lsfRequireDestTag;
    }

    if (bClearRequireDest && (uFlagsIn & lsfRequireDestTag))
    {
        JLOG(j_.trace()) << "Clear lsfRequireDestTag.";
        uFlagsOut &= ~lsfRequireDestTag;
    }

    //
    // DisallowM
    //
    if (bSetDisallowM && !(uFlagsIn & lsfDisallowM))
    {
        JLOG(j_.trace()) << "Set lsfDisallowM.";
        uFlagsOut |= lsfDisallowM;
    }

    if (bClearDisallowM && (uFlagsIn & lsfDisallowM))
    {
        JLOG(j_.trace()) << "Clear lsfDisallowM.";
        uFlagsOut &= ~lsfDisallowM;
    }

    //
    // DisableMaster
    //
    if ((uSetFlag == asfDisableMaster) && !(uFlagsIn & lsfDisableMaster))
    {
        if (!sigWithMaster)
        {
            JLOG(j_.trace()) << "Must use master key to disable master key.";
            return tecNEED_MASTER_KEY;
        }

        if ((!sle->isFieldPresent (sfRegularKey)) &&
            (!view().peek (keylet::signers (account_))))
        {
            // Account has no regular key or multi-signer signer list.

            // Prevent transaction changes until we're ready.
            if (view().rules().enabled(featureMultiSign))
                return tecNO_ALTERNATIVE_KEY;

            return tecNO_REGULAR_KEY;
        }

        JLOG(j_.trace()) << "Set lsfDisableMaster.";
        uFlagsOut |= lsfDisableMaster;
    }

    if ((uClearFlag == asfDisableMaster) && (uFlagsIn & lsfDisableMaster))
    {
        JLOG(j_.trace()) << "Clear lsfDisableMaster.";
        uFlagsOut &= ~lsfDisableMaster;
    }

    //
    // DefaultMTChain
    //
    if (uSetFlag == asfDefaultMtchain)
    {
        uFlagsOut   |= lsfDefaultMtchain;
    }
    else if (uClearFlag == asfDefaultMtchain)
    {
        uFlagsOut   &= ~lsfDefaultMtchain;
    }

    //
    // NoFreeze
    //
    if (uSetFlag == asfNoFreeze)
    {
        if (!sigWithMaster && !(uFlagsIn & lsfDisableMaster))
        {
            JLOG(j_.trace()) << "Can't use regular key to set NoFreeze.";
            return tecNEED_MASTER_KEY;
        }

        JLOG(j_.trace()) << "Set NoFreeze flag";
        uFlagsOut |= lsfNoFreeze;
    }

    // Anyone may set global freeze
    if (uSetFlag == asfGlobalFreeze)
    {
        JLOG(j_.trace()) << "Set GlobalFreeze flag";
        uFlagsOut |= lsfGlobalFreeze;
    }

    // If you have set NoFreeze, you may not clear GlobalFreeze
    // This prevents those who have set NoFreeze from using
    // GlobalFreeze strategically.
    if ((uSetFlag != asfGlobalFreeze) && (uClearFlag == asfGlobalFreeze) &&
        ((uFlagsOut & lsfNoFreeze) == 0))
    {
        JLOG(j_.trace()) << "Clear GlobalFreeze flag";
        uFlagsOut &= ~lsfGlobalFreeze;
    }

    //
    // Track transaction IDs signed by this account in its root
    //
    if ((uSetFlag == asfAccountTxnID) && !sle->isFieldPresent (sfAccountTxnID))
    {
        JLOG(j_.trace()) << "Set AccountTxnID";
        sle->makeFieldPresent (sfAccountTxnID);
        }

    if ((uClearFlag == asfAccountTxnID) && sle->isFieldPresent (sfAccountTxnID))
    {
        JLOG(j_.trace()) << "Clear AccountTxnID";
        sle->makeFieldAbsent (sfAccountTxnID);
    }

    //
    // EmailHash
    //
    if (ctx_.tx.isFieldPresent (sfEmailHash))
    {
        uint128 const uHash = ctx_.tx.getFieldH128 (sfEmailHash);

        if (!uHash)
        {
            JLOG(j_.trace()) << "unset email hash";
            sle->makeFieldAbsent (sfEmailHash);
        }
        else
        {
            JLOG(j_.trace()) << "set email hash";
            sle->setFieldH128 (sfEmailHash, uHash);
        }
    }

    //
    // WalletLocator
    //
    if (ctx_.tx.isFieldPresent (sfWalletLocator))
    {
        uint256 const uHash = ctx_.tx.getFieldH256 (sfWalletLocator);

        if (!uHash)
        {
            JLOG(j_.trace()) << "unset wallet locator";
            sle->makeFieldAbsent (sfWalletLocator);
        }
        else
        {
            JLOG(j_.trace()) << "set wallet locator";
            sle->setFieldH256 (sfWalletLocator, uHash);
        }
    }

    //
    // MessageKey
    //
    if (ctx_.tx.isFieldPresent (sfMessageKey))
    {
        Blob const messageKey = ctx_.tx.getFieldVL (sfMessageKey);

        if (messageKey.empty ())
        {
            JLOG(j_.debug()) << "set message key";
            sle->makeFieldAbsent (sfMessageKey);
        }
        else
        {
            JLOG(j_.debug()) << "set message key";
            sle->setFieldVL (sfMessageKey, messageKey);
        }
    }

    //
    // Domain
    //
    if (ctx_.tx.isFieldPresent (sfDomain))
    {
        Blob const domain = ctx_.tx.getFieldVL (sfDomain);

        if (domain.empty ())
        {
            JLOG(j_.trace()) << "unset domain";
            sle->makeFieldAbsent (sfDomain);
        }
        else
        {
            JLOG(j_.trace()) << "set domain";
            sle->setFieldVL (sfDomain, domain);
        }
    }

    //
    // TransferRate
    //
    if (ctx_.tx.isFieldPresent (sfTransferRate))
    {
        std::uint32_t uRate = ctx_.tx.getFieldU32 (sfTransferRate);

        if (uRate == 0 || uRate == QUALITY_ONE)
        {
            JLOG(j_.trace()) << "unset transfer rate";
            sle->makeFieldAbsent (sfTransferRate);
        }
        else if (uRate > QUALITY_ONE)
        {
            JLOG(j_.trace()) << "set transfer rate";
            sle->setFieldU32 (sfTransferRate, uRate);
        }
    }

    if (ctx_.tx.isFieldPresent (sfTransferFee))
    {
        STAmount const& tf = ctx_.tx.getFieldAmount (sfTransferFee);
        if (!tf)
        {
            JLOG(j_.warn()) << "unset transfer fee";
            sle->makeFieldAbsent (sfTransferFee);
        }
        else
        {
            JLOG(j_.warn()) << "set transfer fee: " << tf;
            sle->setFieldAmount(sfTransferFee, tf);
        }
    }

    //
    // TickSize
    //
    if (ctx_.tx.isFieldPresent (sfTickSize))
    {
        auto uTickSize = ctx_.tx[sfTickSize];
        if ((uTickSize == 0) || (uTickSize == Quality::maxTickSize))
        {
            JLOG(j_.trace()) << "unset tick size";
            sle->makeFieldAbsent (sfTickSize);
        }
        else
        {
            JLOG(j_.trace()) << "set tick size";
            sle->setFieldU8 (sfTickSize, uTickSize);
        }
    }

    if (uFlagsIn != uFlagsOut)
        sle->setFieldU32 (sfFlags, uFlagsOut);

    #define MAX_ENTRY_NUM_PER_ACCOUNT 128
    if (ctx_.tx.isFieldPresent (sfLimitAmount))
    {
        auto const& limit = ctx_.tx.getFieldAmount (sfLimitAmount);
        if (isM(limit) || limit.getIssuer() != account_)
        {
            return tecNO_PERMISSION;
        }

        if (limit.negative ())
        {
            // 是否考虑: 如果当前二级资产在外流通数量为0, 则允许撤销限制
            return tecNO_PERMISSION;
        }

        // 如果要设置的limit小于当前流通总量
        auto total = getIssueTotalAmount (limit.issue(), view(), j_);
        JLOG(j_.warn()) << "limit = " << limit.getFullText()
                        << ", total = " << total.getFullText();
        if (limit < total)
        {
            return tecNO_PERMISSION;
        }

        STVector256 limits;
        auto k = keylet::line(mAccount(), limit.issue());
        auto sleLimit = view().peek(k);
        if (sleLimit)
        {
            // 如果当前在外流通的数量为0, 则允许修改上限
            if (sleLimit->isFieldPresent(sfLimitAmount) && total.signum() > 0)
                return tecNO_PERMISSION; // 不允许重复设置发行资产上限

            if (limit.signum () > 0)
            {
                view().update (sleLimit);
                sleLimit->setFieldAmount (sfLimitAmount, limit);
            }
            else //if (sleLimit->isFieldPresent(sfLimitAmount))
            {
                static auto f = [](SLE::ref sle) {
                    if (sle->isFieldPresent(sfLimitAmount))
		    {
                        sle->makeFieldAbsent(sfLimitAmount);
                        return true;
                    }

                    return false;
                };

                auto ter = delIssueInfo(limit, view(), sle, f, j_);
                if (ter != tesSUCCESS) return ter;
	        /*
                sleLimit->makeFieldAbsent(sfLimitAmount);
                if (canDeleteSLE(sleLimit))
                {
                    view().erase (sleLimit);
                    if (sle->isFieldPresent (sfIssues))
                    {
                        limits.setValue(sle->getFieldV256 (sfIssues));
                        auto itr = std::find(limits.begin(), limits.end(), k.key);
                        if (itr != limits.end())
                        {
                            limits.erase(itr);
                            if (limits.empty())
                            {
                                sle->makeFieldAbsent (sfIssues);
                            }
                            else
                            {
                                sle->setFieldV256 (sfIssues, limits);
                            }
                        }
                        else
                        {
                            JLOG(j_.error()) << "It shouldn't get here when delete limit for "
                                             << to_string(limit.issue());
                        }
                    }
                }
                else
                {
                    view().update (sleLimit);
                }
                */
            }
        }
        else if (limit.signum() > 0)
        {
            TER ter;
            std::tie(ter, std::ignore) = addIssueInfo(limit, view(), sleLimit, sle, j_);
            if (ter != tesSUCCESS) return ter;

	    /* 
            if (sle->isFieldPresent (sfIssues))
            {
                limits.setValue(sle->getFieldV256 (sfIssues));
            }

            if (limits.size() >= MAX_ENTRY_NUM_PER_ACCOUNT) // 超过了允许的最大条目数
            {
                return tecDIR_FULL;
            }

            sleLimit = std::make_shared<SLE>(k);
            if (!sleLimit) // 失败分配条目来保存二级资产发行上限
            {
                JLOG(j_.error()) << "failed to alloc entry for limit " << limit.getFullText();
                return tecNO_ENTRY;
            }

            view().insert (sleLimit);
            if (std::find(limits.begin(), limits.end(), k.key) == limits.end())
            {
                limits.push_back(k.key);
                sle->setFieldV256 (sfIssues, limits);
            }
            else
            {
                JLOG(j_.error()) << "It shouldn't get here when set transfer limit for " <<
                    limit.getFullText();
            }

            sleLimit->setFieldAmount (sfLowLimit,    limit);
            sleLimit->setFieldAmount (sfHighLimit,   limit);
	    */
            sleLimit->setFieldAmount (sfLimitAmount, limit);
        }
    }

    if (ctx_.tx.isFieldPresent (sfTxnRate))
    {
        auto const& saRate = ctx_.tx.getFieldAmount (sfTxnRate);
        if (isM(saRate) || saRate.negative() || saRate.getIssuer() != account_)
        {
            return tecNO_PERMISSION;
        }

        STVector256 rates;
        bool updateVec = false;
        if (sle->isFieldPresent (sfIssues))
        {
            rates.setValue(sle->getFieldV256 (sfIssues));
        }

        auto strIssue = to_string(saRate.issue());
        auto rate = STAmount(mIssue(), saRate.mantissa(), saRate.exponent()).mantissa();
        auto k = keylet::line(mAccount(), saRate.issue());
        auto sleRate = view().peek(k);
        if (rate == 0 || rate == QUALITY_ONE)
        {
            if (sleRate)
            {
                JLOG(j_.warn()) << "unset " << strIssue << " transfer rate!";
                static auto f = [](SLE::ref sle) {
                    if (sle->isFieldPresent(sfTransferRate))
                    {
                        sle->makeFieldAbsent(sfTransferRate);
                        return true;
                    }

                    return false;
                };
                auto ter = delIssueInfo(saRate, view(), sle, f, j_);
                if (ter != tesSUCCESS) return ter;

		/*
                sleRate->makeFieldAbsent(sfTransferRate);
                if (canDeleteSLE(sleRate))
                {
                    JLOG(j_.warn()) << "delete transfer rate for " << strIssue;
                    view().erase(sleRate);
                    auto itr = std::find(rates.begin(), rates.end(), k.key);
                    if (itr != rates.end())
                    {
                        rates.erase(itr);
                        updateVec = true;
                    }
                    else
                    {
                        JLOG(j_.error()) << "It shouldn't get here when delete transfer rate for " << strIssue;
                    }
                }
                else
                {
                    view().update(sleRate);
                }
		*/
            }
        }
        else if (rate > QUALITY_ONE && rate <= (QUALITY_ONE * 2))
        {

            if (!sleRate)
            {
                TER ter;
                std::tie(ter, std::ignore) = addIssueInfo(saRate, view(), sleRate, sle, j_);
                if (ter != tesSUCCESS) return ter;

		/*
                if (rates.size() >= MAX_ENTRY_NUM_PER_ACCOUNT)
                {
                    return tecDIR_FULL;
                }

                sleRate = std::make_shared<SLE>(k);
                if (!sleRate)
                {
                    JLOG(j_.error()) << "failed to set " << strIssue << " transfer rate: "
                                     << rate;
                    return tecNO_ENTRY;
                }

                JLOG(j_.warn()) << "set " << strIssue << " transfer rate: " << rate;
                view().insert(sleRate);
                if (std::find(rates.begin(), rates.end(), k.key) == rates.end())
                {
                    rates.push_back(k.key);
                    updateVec = true;
                }
                else
                {
                    JLOG(j_.error()) << "It shouldn't get here when add transfer rate for "
                                     << strIssue;
                }
		*/
            }
            else if (sleRate->getFieldU32(sfTransferRate) != rate)
            {
                JLOG(j_.warn()) << "update " << strIssue << " transfer rate from "
                                << sleRate->getFieldU32(sfTransferRate) << " to " << rate;
                view().update(sleRate);
            }

            //sleRate->setFieldAmount(sfLowLimit,  saRate);
            //sleRate->setFieldAmount(sfHighLimit, saRate);
            sleRate->setFieldU32(sfTransferRate, rate);
        }
        else
        {
            return tecFAILED_PROCESSING;
        }

        if (updateVec)
        {
            if (!rates.empty())
            {
                sle->setFieldV256(sfIssues, rates);
            }
            else if (sle->isFieldPresent(sfIssues))
            {
                sle->makeFieldAbsent(sfIssues);
            }
        }
    }

    if (uSetFlag == asfIOUPrecision)
    {
        return setIOUPrecision(sle);
    }

    return tesSUCCESS;
}

TER
delIssueInfo (STAmount const& amount, ApplyView& view, std::shared_ptr<SLE> const& sleOwner,
              std::function<bool (SLE::ref)> f, beast::Journal j)
{
    auto const& issue = amount.issue();
    auto k = keylet::line(mAccount(), issue);
    auto sleIssue = view.peek(k);
    if (!sleIssue) return tecNO_ENTRY;

    bool change = f(sleIssue);
    if (canDeleteSLE(sleIssue))
    {
        if (sleIssue->isFieldPresent(sfLowNode))
        {
            auto uLowNode = sleIssue->getFieldU64(sfLowNode);


            auto ter = dirDelete(view, false, uLowNode, getOwnerDirIndex(~issue.account),
                sleIssue->key(), false, !uLowNode, j);

            if (ter != tesSUCCESS) return ter;
            adjustOwnerCount(view, sleOwner, -1, j);
	}

        view.erase(sleIssue);
        if (sleOwner->isFieldPresent (sfIssues))
        {
            STVector256 issues;
            issues.setValue(sleOwner->getFieldV256 (sfIssues));
            auto it = std::find(issues.begin(), issues.end(), k.key);
            if (it != issues.end())
            {
                issues.erase(it);
                if (issues.empty())
                {
                    sleOwner->makeFieldAbsent (sfIssues);
                }
                else
                {
                    sleOwner->setFieldV256 (sfIssues, issues);
                }
            }
	}

    }
    else if (change)
    {
        view.update(sleIssue);
    }

    return tesSUCCESS;
}
  
TER SetAccount::setIOUPrecision (std::shared_ptr<SLE> const& sleAccount)
{
    auto const& amount = ctx_.tx.getFieldAmount (sfAmount);

    if (amount.getIssuer() != account_)
    {
        return tecNO_PERMISSION;
    }

    if (amount.signum() < 0)
    {
        static auto f = [](SLE::ref sle) {
            if (sle->isFieldPresent(sfHighOut))
            {
                sle->makeFieldAbsent(sfHighOut);
                return true;
            }

            return false;
        };
        return delIssueInfo(amount, view(), sleAccount, f, j_);
    }

    TER ter;
    bool insert;
    std::shared_ptr<SLE> sleIssue;
    std::tie (ter, insert) = addIssueInfo(amount, view(), sleIssue, sleAccount, j_);
    if (ter != tesSUCCESS) return ter;

    auto const& precision = sleIssue->getFieldAmount(sfHighOut);
    if ((!precision.native() && precision.decimal() <= amount.decimal()) ||
        !getIssueTotalAmount(amount.issue(), view(), j_))
    {
        sleIssue->setFieldAmount(sfHighOut, amount);
        if (!insert)
        {
            view().update(sleIssue);
        }
    }
    else
    {
        ter = tecNO_PERMISSION;
    }

    return ter;
}

}
