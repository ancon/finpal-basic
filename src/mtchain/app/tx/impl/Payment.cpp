//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/Payment.h>
#include <mtchain/app/paths/MtchainCalc.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/protocol/st.h>
#include <mtchain/protocol/TxFlags.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/json/json_reader.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/basics/random.h>

namespace mtchain {

// See https://mtchain.io/wiki/Transaction_Format#Payment_.280.29

static std::vector<std::pair<AccountID, STAmount const& > > getAccountList (STTx const& tx)
{
    std::vector<std::pair<AccountID, STAmount const& > > accounts;

    if (!tx.isFieldPresent(sfPayees))
    {
        accounts.emplace_back (std::piecewise_construct,
                               std::forward_as_tuple(tx.getAccountID(sfDestination)),
                               std::forward_as_tuple(tx.getFieldAmount(sfAmount)));
    }
    else
    {
        auto const& payees = tx.getFieldArray (sfPayees);
        for (auto const& payee : payees)
        {
            auto const& amount = payee.isFieldPresent(sfAmount) ? payee.getFieldAmount(sfAmount) : tx.getFieldAmount(sfAmount);
            accounts.emplace_back (std::piecewise_construct,
                                   std::forward_as_tuple(payee.getAccountID(sfDestination)),
                                   std::forward_as_tuple(amount));
        }
    }

    return std::move (accounts);
}

static std::vector<std::tuple<AccountID, STAmount const&, STObject & > > getAccountListEx(STTx &tx)
{
    std::vector<std::tuple<AccountID, STAmount const&, STObject & > > accounts;

    if (!tx.isFieldPresent(sfPayees))
    {
        accounts.emplace_back (std::forward_as_tuple(tx.getAccountID(sfDestination),
                                                     tx.getFieldAmount(sfAmount), tx));
    }
    else
    {
        auto & payees = tx.peekFieldArray (sfPayees);
        for (auto & payee : payees)
        {
            auto const& amount = payee.isFieldPresent(sfAmount) ? payee.getFieldAmount(sfAmount) : tx.getFieldAmount(sfAmount);
            accounts.emplace_back (std::forward_as_tuple(payee.getAccountID(sfDestination),
                                                         amount, payee));

        }
    }

    return std::move (accounts);
}

MAmount Payment::calculateMaxSpend(STTx const& tx)
{
    if (tx.isFieldPresent(sfSendMax))
    {
        auto const& sendMax = tx[sfSendMax];
        return sendMax.native() ? sendMax.m() : beast::zero;
    }

    /* If there's no sfSendMax in M, and the sfAmount isn't
    in M, then the transaction can not send M. */
    //    auto const& saDstAmount = tx.getFieldAmount(sfAmount);
    //return saDstAmount.native() ? saDstAmount.m() : beast::zero;
    auto const accounts = getAccountList(tx);
    MAmount saDstAmount = beast::zero;
    for (auto const& account : accounts)
    {
        saDstAmount += account.second.native() ? account.second.m() : beast::zero;
    }

    return saDstAmount;
}

TER
Payment::preflight (PreflightContext const& ctx)
{
    TER terResult = tecNO_DST;
    auto const accounts = getAccountList(ctx.tx);
    for (auto const& account : accounts)
    {
        terResult = preflight(ctx, account.first, account.second);
        if (terResult != tesSUCCESS)
        {
            break;
        }
    }

    return terResult;
}

TER
Payment::preflight (PreflightContext const& ctx, AccountID const& uDstAccountID, STAmount const& saDstAmount)
{
    auto const ret = preflight1 (ctx);
    if (!isTesSuccess (ret))
        return ret;

    auto& tx = ctx.tx;
    auto& j = ctx.j;

    std::uint32_t const uTxFlags = tx.getFlags ();

    if (uTxFlags & tfPaymentMask)
    {
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Invalid flags set.";
        return temINVALID_FLAG;
    }

    bool const partialPaymentAllowed = uTxFlags & tfPartialPayment;
    bool const limitQuality = uTxFlags & tfLimitQuality;
    bool const defaultPathsAllowed = !(uTxFlags & tfNoMTChainDirect);
    bool const bPaths = tx.isFieldPresent (sfPaths);
    bool const bMax = tx.isFieldPresent (sfSendMax);

    STAmount maxSourceAmount;
    auto const uSrcAccountID = tx.getAccountID(sfAccount);

    if (bMax)
        maxSourceAmount = tx.getFieldAmount (sfSendMax);
    else if (saDstAmount.native ())
        maxSourceAmount = saDstAmount;
    else
        maxSourceAmount = STAmount (
            { saDstAmount.getCurrency (), uSrcAccountID },
            saDstAmount.mantissa(), saDstAmount.exponent (),
            saDstAmount < zero);

    auto const& uSrcCurrency = maxSourceAmount.getCurrency ();
    auto const& uDstCurrency = saDstAmount.getCurrency ();

    // isZero() is M.  FIX!
    bool const bMDirect = uSrcCurrency.isZero () && uDstCurrency.isZero ();

    if (!isLegalNet (saDstAmount) || !isLegalNet (maxSourceAmount))
        return temBAD_AMOUNT;


    if (!uDstAccountID)
    {
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Payment destination account not specified.";
        return temDST_NEEDED;
    }
    if (bMax && maxSourceAmount <= zero)
    {
        JLOG(j.trace()) << "Malformed transaction: " <<
            "bad max amount: " << maxSourceAmount.getFullText ();
        return temBAD_AMOUNT;
    }
    if (saDstAmount <= zero)
    {
        JLOG(j.trace()) << "Malformed transaction: "<<
            "bad dst amount: " << saDstAmount.getFullText ();
        return temBAD_AMOUNT;
    }
    if (badCurrency() == uSrcCurrency || badCurrency() == uDstCurrency)
    {
        JLOG(j.trace()) <<"Malformed transaction: " <<
            "Bad currency.";
        return temBAD_CURRENCY;
    }
    if (uSrcAccountID == uDstAccountID && uSrcCurrency == uDstCurrency && !bPaths)
    {
        // You're signing yourself a payment.
        // If bPaths is true, you might be trying some arbitrage.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Redundant payment from " << to_string (uSrcAccountID) <<
            " to self without path for " << to_string (uDstCurrency);
        return temREDUNDANT;
    }
    if (bMDirect && bMax)
    {
        // Consistent but redundant transaction.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "SendMax specified for M to M.";
        return temBAD_SEND_M_MAX;
    }
    if (bMDirect && bPaths)
    {
        // M is sent without paths.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Paths specified for M to M.";
        return temBAD_SEND_M_PATHS;
    }
    if (bMDirect && partialPaymentAllowed)
    {
        // Consistent but redundant transaction.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Partial payment specified for M to M.";
        return temBAD_SEND_M_PARTIAL;
    }
    if (bMDirect && limitQuality)
    {
        // Consistent but redundant transaction.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "Limit quality specified for M to M.";
        return temBAD_SEND_M_LIMIT;
    }
    if (bMDirect && !defaultPathsAllowed)
    {
        // Consistent but redundant transaction.
        JLOG(j.trace()) << "Malformed transaction: " <<
            "No mtchain direct specified for M to M.";
        return temBAD_SEND_M_NO_DIRECT;
    }

    auto const deliverMin = tx[~sfDeliverMin];
    if (deliverMin)
    {
        if (! partialPaymentAllowed)
        {
            JLOG(j.trace()) << "Malformed transaction: Partial payment not "
                "specified for " << jss::DeliverMin.c_str() << ".";
            return temBAD_AMOUNT;
        }

        auto const dMin = *deliverMin;
        if (!isLegalNet(dMin) || dMin <= zero)
        {
            JLOG(j.trace()) << "Malformed transaction: Invalid " <<
                jss::DeliverMin.c_str() << " amount. " <<
                    dMin.getFullText();
            return temBAD_AMOUNT;
        }
        if (dMin.issue() != saDstAmount.issue())
        {
            JLOG(j.trace()) <<  "Malformed transaction: Dst issue differs "
                "from " << jss::DeliverMin.c_str() << ". " <<
                    dMin.getFullText();
            return temBAD_AMOUNT;
        }
        if (dMin > saDstAmount)
        {
            JLOG(j.trace()) << "Malformed transaction: Dst amount less than " <<
                jss::DeliverMin.c_str() << ". " <<
                    dMin.getFullText();
            return temBAD_AMOUNT;
        }
    }

    return preflight2 (ctx);
}

TER
Payment::preclaim (PreclaimContext const& ctx, AccountID const& uDstAccountID, STAmount const& saDstAmount)
{  
    // Mtchain if source or destination is non-native or if there are paths.
    std::uint32_t const uTxFlags = ctx.tx.getFlags();
    bool const partialPaymentAllowed = uTxFlags & tfPartialPayment;
    auto const paths = ctx.tx.isFieldPresent(sfPaths);
    auto const sendMax = ctx.tx[~sfSendMax];

    auto const k = keylet::account(uDstAccountID);
    auto const sleDst = ctx.view.read(k);

    if (!sleDst)
    {
        // Destination account does not exist.
        if (!saDstAmount.native())
        {
            JLOG(ctx.j.trace()) <<
                "Delay transaction: Destination account does not exist.";

            // Another transaction could create the account and then this
            // transaction would succeed.
            return tecNO_DST;
        }
        else if (ctx.view.open()
            && partialPaymentAllowed)
        {
            // You cannot fund an account with a partial payment.
            // Make retry work smaller, by rejecting this.
            JLOG(ctx.j.trace()) <<
                "Delay transaction: Partial payment not allowed to create account.";


            // Another transaction could create the account and then this
            // transaction would succeed.
            return telNO_DST_PARTIAL;
        }
        else if (saDstAmount < STAmount(ctx.view.fees().accountReserve(0)))
        {
            // accountReserve is the minimum amount that an account can have.
            // Reserve is not scaled by load.
            JLOG(ctx.j.trace()) <<
                "Delay transaction: Destination account does not exist. " <<
                "Insufficent payment to create account.";

	    std::cout << saDstAmount << "" << STAmount(ctx.view.fees().accountReserve(0)) << std::endl;
            // TODO: dedupe
            // Another transaction could create the account and then this
            // transaction would succeed.
            return tecNO_DST_INSUF_M;
        }
    }
    else if ((sleDst->getFlags() & lsfRequireDestTag) &&
        !ctx.tx.isFieldPresent(sfDestinationTag))
    {
        // The tag is basically account-specific information we don't
        // understand, but we can require someone to fill it in.

        // We didn't make this test for a newly-formed account because there's
        // no way for this field to be set.
        JLOG(ctx.j.trace()) << "Malformed transaction: DestinationTag required.";

        return tecDST_TAG_NEEDED;
    }

    if (paths || sendMax || !saDstAmount.native())
    {
        // Mtchain payment with at least one intermediate step and uses
        // transitive balances.

        // Copy paths into an editable class.
        STPathSet const spsPaths = ctx.tx.getFieldPathSet(sfPaths);

        auto pathTooBig = spsPaths.size() > MaxPathSize;

        if(!pathTooBig)
            for (auto const& path : spsPaths)
                if (path.size() > MaxPathLength)
                {
                    pathTooBig = true;
                    break;
                }

        if (ctx.view.open() && pathTooBig)
        {
            return telBAD_PATH_COUNT; // Too many paths for proposed ledger.
        }
    }

    return tesSUCCESS;
}

TER
Payment::preclaim (PreclaimContext const& ctx)
{
    TER terResult = tecNO_DST;
    auto const accounts = getAccountList (ctx.tx);
    for (auto const& account : accounts)
    {
        terResult = preclaim (ctx, account.first, account.second);
        if (terResult != tesSUCCESS)
        {
            break;
        }
    }

    return terResult;
}

TER
Payment::doApply ()
{
    TER terResult = tecNO_DST;
    STTx &tx = const_cast<STTx &>(ctx_.tx);
    auto accounts = getAccountListEx(tx);
    for (auto & account : accounts)
    {
        auto const& saDstAmount = std::get<1>(account);

        terResult = doApply (std::get<0>(account), saDstAmount, std::get<2>(account));
        if (terResult != tesSUCCESS)
        {
            break;
	}

        if (saDstAmount.native ())
        {
            mPriorBalance  -= saDstAmount.m();
            mSourceBalance -= saDstAmount.m();
        }
    }

    return terResult;
}

TER
Payment::doApply (AccountID const& uDstAccountID, STAmount const& saDstAmount, STObject &obj)
{
    auto k = keylet::line(mAccount(), saDstAmount.issue());
    auto sleIssue = view().read(k);
    if (sleIssue)
    {
        auto const& precision = sleIssue->getFieldAmount(sfHighOut);
        if (!precision.native() && precision.decimal() < saDstAmount.decimal())
        {
            return tecIOU_PRECISION_MISMATCH;
        }

        sleIssue.reset();
    }

    auto const deliverMin = ctx_.tx[~sfDeliverMin];

    // Mtchain if source or destination is non-native or if there are paths.
    std::uint32_t const uTxFlags = ctx_.tx.getFlags ();
    bool const partialPaymentAllowed = uTxFlags & tfPartialPayment;
    bool const limitQuality = uTxFlags & tfLimitQuality;
    bool const defaultPathsAllowed = !(uTxFlags & tfNoMTChainDirect);
    auto const paths = ctx_.tx.isFieldPresent(sfPaths);
    auto const sendMax = ctx_.tx[~sfSendMax];

    STAmount maxSourceAmount;
    if (sendMax)
        maxSourceAmount = *sendMax;
    else if (saDstAmount.native ())
        maxSourceAmount = saDstAmount;
    else
        maxSourceAmount = STAmount (
            {saDstAmount.getCurrency (), account_},
            saDstAmount.mantissa(), saDstAmount.exponent (),
            saDstAmount < zero);

    JLOG(j_.trace()) << "uDstAccountID=" << toBase58(uDstAccountID) <<
         "maxSourceAmount=" << maxSourceAmount.getFullText () <<
         " saDstAmount=" << saDstAmount.getFullText ();

    // Open a ledger for editing.
    k = keylet::account(uDstAccountID);
    SLE::pointer sleDst = view().peek (k);

    if (!sleDst)
    {
        // Create the account.
        sleDst = std::make_shared<SLE>(k);
        sleDst->setAccountID(sfAccount, uDstAccountID);
        sleDst->setFieldU32(sfSequence, 1);
        view().insert(sleDst);
    }
    else
    {
        // Tell the engine that we are intending to change the destination
        // account.  The source account gets always charged a fee so it's always
        // marked as modified.
        view().update (sleDst);
    }

    TER terResult;

    // if transfer issued money through rippling
    bool const bMTChain = paths || sendMax || !saDstAmount.native ();
    // XXX Should sendMax be sufficient to imply mtchain?

    if (bMTChain)
    {
        // Mtchain payment with at least one intermediate step and uses
        // transitive balances.

        // Copy paths into an editable class.
        STPathSet const& spsPaths = ctx_.tx.getFieldPathSet (sfPaths);

        path::MtchainCalc::Input rcInput;
        rcInput.partialPaymentAllowed = partialPaymentAllowed;
        rcInput.defaultPathsAllowed = defaultPathsAllowed;
        rcInput.limitQuality = limitQuality;
        rcInput.isLedgerOpen = view().open();

        std::vector<Issue> issues;
        std::vector<STAmount> totals;
            issues.push_back(saDstAmount.issue());
        if (sendMax && maxSourceAmount.issue() != saDstAmount.issue())
        {
            issues.push_back(maxSourceAmount.issue());
        }
        for (auto const& issue : issues)
        {
	    auto const limit = getIssueLimitAmount(issue, view());
            totals.push_back(limit ? getIssueTotalAmount (issue, view(), j_) : issue);
        }

        path::MtchainCalc::Output rc;
        {
            PaymentSandbox pv(&view());
            JLOG(j_.debug())
                << "Entering MtchainCalc in payment: " << ctx_.tx.getTransactionID();
            rc = path::MtchainCalc::mtchainCalculate (
                pv,
                maxSourceAmount,
                saDstAmount,
                uDstAccountID,
                account_,
                spsPaths,
                ctx_.app.logs(),
                &rcInput);
            // VFALCO NOTE We might not need to apply, depending
            //             on the TER. But always applying *should*
            //             be safe.
            pv.apply(ctx_.rawView());
            if (rc.fee)
                addTxFee (ctx_.tx, *rc.fee);
        }

        // TODO: is this right?  If the amount is the correct amount, was
        // the delivered amount previously set?
        if (rc.result () == tesSUCCESS &&
            rc.actualAmountOut != saDstAmount)
        {
            if (deliverMin && rc.actualAmountOut <
                *deliverMin)
                rc.setResult (tecPATH_PARTIAL);
            else
                ctx_.deliver (rc.actualAmountOut);
        }

        terResult = rc.result ();

        // Because of its overhead, if MtchainCalc
        // fails with a retry code, claim a fee
        // instead. Maybe the user will be more
        // careful with their path spec next time.
        if (isTerRetry (terResult))
            terResult = tecPATH_DRY;

        if (terResult == tesSUCCESS)
        {
            for (auto i = 0; i < issues.size(); ++i)
            {
                auto const& issue = issues[i];
                auto const limit = getIssueLimitAmount(issue, view());
                if (limit)
                {
                    auto const total = getIssueTotalAmount (issue, view());
                    if (total > totals[i] && total > *limit)
                        return tecOVERISSUE;
                }
            }
        }
    }
    else
    {
        assert (saDstAmount.native ());

        // Direct M payment.

        // uOwnerCount is the number of entries in this legder for this
        // account that require a reserve.
        auto const uOwnerCount = view().read(
            keylet::account(account_))->getFieldU32 (sfOwnerCount);

        // This is the total reserve in drops.
        auto const reserve = view().fees().accountReserve(uOwnerCount);

        // mPriorBalance is the balance on the sending account BEFORE the
        // fees were charged. We want to make sure we have enough reserve
        // to send. Allow final spend to use reserve for fee.
        auto const mmm = std::max(reserve,
            ctx_.tx.getFieldAmount (sfFee).m ());

        if (mPriorBalance < saDstAmount.m () + mmm)
        {
            // Vote no. However the transaction might succeed, if applied in
            // a different order.
            JLOG(j_.trace()) << "Delay transaction: Insufficient funds: " <<
                " " << to_string (mPriorBalance) <<
                " / " << to_string (saDstAmount.m () + mmm) <<
                " (" << to_string (reserve) << ")";

            terResult = tecUNFUNDED_PAYMENT;
        }
        else
        {
            // The source account does have enough money, so do the
            // arithmetic for the transfer and make the ledger change.
            view().peek(keylet::account(account_))->setFieldAmount (sfBalance,
                mSourceBalance - saDstAmount);
            sleDst->setFieldAmount (sfBalance,
                sleDst->getFieldAmount (sfBalance) + saDstAmount);

            // Re-arm the password change fee if we can and need to.
            if ((sleDst->getFlags () & lsfPasswordSpent))
                sleDst->clearFlag (lsfPasswordSpent);

            terResult = tesSUCCESS;
        }
    }

    return terResult;
}

#ifdef IPFS_ENABLE
long get_file_size(std::string const& filename)
{
    FILE *pFile  = fopen (filename.c_str(), "r");
    long size = -1;
    if (pFile)
    {
        if (fseek(pFile, 0, SEEK_END) == 0)
        {
            size = ftell(pFile);
        }

        fclose(pFile);
    }

    return size;
}

static inline std::string get_file_path(std::string const& filename, Application& app)
{
    return app.config().FILE_UPLOAD_DIR + filename;
}

boost::optional<Json::Value> parseExtraInfo(STTx const& tx, beast::Journal const& j)
{
    if (tx.isFieldPresent (sfMemos))
    {
        static auto const fileDownloadType = to_blob(jss::FileDownload);
        static auto const fileUploadType = to_blob(jss::FileUpload);
        static auto const jsonFormat = to_blob("JSON");

        auto const& memos = tx.getFieldArray (sfMemos);
        for (auto const& memo : memos) {
            auto type = memo.getFieldVL (sfMemoType);
            if (type != fileDownloadType && type != fileUploadType)
                continue;

            auto format = memo.getFieldVL (sfMemoFormat);
            if (format != jsonFormat)
                continue;

            Json::Reader reader;
            Json::Value filesInfo;
            auto data = memo.getFieldVL (sfMemoData);
            auto begin = (const char *)data.data ();
            if (!reader.parse (begin, begin + data.size(), filesInfo)) {
                break;
            }

            filesInfo[jss::type] = to_string(type);
            JLOG (j.warn()) << to_string(type) << " Info: " << to_string(filesInfo);
            return filesInfo;
        }
    }

    return boost::none;
}

bool checkFilesInfo(STTx const& tx, Json::Value const& filesInfo, bool local, bool upload,
                    std::string &filePath, Application &app)
{
    if (!filesInfo.isMember(jss::files))
    {
        return false;
    }

    auto &files = filesInfo[jss::files];
    if (!files.isArray() || files.size() < 1)
    {
        return false;
    }

    auto &file = files[0U];
    if (!file.isMember(jss::id) || !file.isMember(jss::size) ||
        !file.isMember(jss::name) || !file.isMember(jss::md5))
    {
        return false;
    }

    AccountID chargeAccount;
    std::string fee; // 分数表示, 左边为Drop数, 右边为Byte数
    auto fileSize = file[jss::size].asUInt();
    if (local)
    {
        chargeAccount = app.getFileChargeAccount(); // 获取本地配置的文件操作支付账号
        fee = app.getFileChargeFee(upload); // 获取本地配置的存储费用信息
        filePath = get_file_path(file[jss::id].asString(), app);
        if (fileSize != get_file_size(filePath))
        {
            return false;
        }
    }
    else
    {
        if (!tx.meta.isFieldPresent(sfChargeAccount))
        {
            return false;
        }

        auto const& sfFileOpFee = upload ? sfStoreFee : sfDownloadFee;
        if (!tx.meta.isFieldPresent(sfFileOpFee))
        {
            return false;
        }

        chargeAccount = tx.meta.getAccountID(sfChargeAccount);
        fee = "1/" + std::to_string(tx.meta.getFieldU32(sfFileOpFee));
    }

    if (filesInfo.isMember(jss::charge_account) &&
	filesInfo[jss::charge_account].asString() != toBase58(chargeAccount))
    {
	return false;
    }

    if (chargeAccount != tx.getAccountID(sfDestination))
    {
        return false;
    }

    if (filesInfo.isMember(jss::fee) && filesInfo[jss::fee].asString() != fee)
    {
        return false;
    }

    if (multiply(parse_fraction(fee), fileSize, mIssue()) > tx.getFieldAmount(sfAmount))
    {
        return false;
    }

    return true;
}

void Payment::preApply(TER terResult)
{
    if (terResult != tesSUCCESS)
    {
        return;
    }

    auto const& tx = ctx_.tx;
    auto extra = parseExtraInfo(tx, j_);
    if (!extra)
    {
        return;
    }

    auto filesInfo = *extra;
    auto fileOpType =  filesInfo[jss::type].asString();
    if (fileOpType == jss::FileDownload)
    {
        static auto const valid_period = 3600;
        auto expiration = view().parentCloseTime().time_since_epoch().count() + valid_period;
        tx.meta.setFieldU32(sfExpiration, expiration);

        tx.meta.setAccountID(sfChargeAccount, ctx_.app.getFileChargeAccount());
        std::uint64_t drops, bytes;
        std::tie(drops, bytes) = parse_fraction(ctx_.app.getFileChargeFee(true));
        auto store_fee = drops ? bytes / drops : 0; // unit: Bytes per Drop

        std::tie(drops, bytes) = parse_fraction(ctx_.app.getFileChargeFee(false));
        auto download_fee = drops ? bytes / drops : 0;
        tx.meta.setFieldU32(sfStoreFee, store_fee);
        tx.meta.setFieldU32(sfDownloadFee, download_fee);

        JLOG(j_.warn()) << "Downloading file will expire after "
                        << to_string(NetClock::time_point(NetClock::duration(expiration)));
    }
}

void Payment::afterApply(ReadView const& view, STTx const& tx, TER terResult,
                         Application& app, beast::Journal& j)
{
    if (!tx.isLocal() || terResult != tesSUCCESS)
    {
        return;
    }

    auto extra = parseExtraInfo(tx, j);
    if (!extra)
    {
        return;
    }

    auto filesInfo = *extra;
    auto fileOpType =  filesInfo[jss::type].asString();
    if (fileOpType == jss::FileUpload)
    {
        std::string localFilePath;
        if (!checkFilesInfo (tx, filesInfo, true, true, localFilePath, app))
        {
            JLOG(j.warn()) << "Upload files info error: " << to_string(filesInfo);
            tx.addon[jss::FileUpload] = "failed";
            return;
        }

        auto &file = filesInfo[jss::files][0U];
        auto fileId = file[jss::id].asString();
        auto filename = file[jss::name].asString();

        ipfs::Json result;
        auto client = app.createIpfsClient();
        if (!client || app.storeFile(client, localFilePath, filename, result) < 0)
        {
            JLOG(j.warn()) << "add file " << filename << "("
                           << fileId << ") to ipfs failed!";

            tx.addon[jss::FileUpload] = "failed";
        }
        else
        {
            JLOG(j.warn()) << "add file " << filename << "("
                           << fileId << ") to ipfs ok:\n" << result.dump(4);

            tx.addon[jss::FileUpload] = "success";

            // delete local file that have added into ipfs
            if (remove(localFilePath.c_str()) != 0)
            {
                JLOG(j.warn()) << "failed to delete local file '" << localFilePath
                               << "' that's uploaded: " << strerror(errno);
            }

            // TODO: if file hash is different from fileId, do what?
            /*
            JLOG(j.warn()) << "ipfs result: " <<
                           << "hash = " << result[0][jss::hash.c_str()].get<std::string>()
                           << ", size = " << file[jss::size].asUInt();
            */
        }
    } else if (fileOpType == jss::FileDownload)
    {
        tx.addon[jss::password] = to_string(rand_uint256());
    }
}

#else
void Payment::preApply(TER terResult)
{
}

void Payment::afterApply(ReadView const& view, STTx const& tx, TER terResult,
                         Application& app, beast::Journal& j)
{

}
#endif

}  // namespace mtchain
