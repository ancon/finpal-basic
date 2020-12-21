//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/ledger/TransactionMaster.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/app/misc/Transaction.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#ifdef IPFS_ENABLE
#include <mtchain/server/impl/JSONRPCUtil.h>
#include <mtchain/protocol/BuildInfo.h>
#endif

namespace mtchain {

// {
//   transaction: <hex>
// }

static inline
bool
isHexTxID (std::string const& txid)
{
    return isHex64(txid);
}

static
bool
isValidated (RPC::Context& context, std::uint32_t seq, uint256 const& hash)
{
    if (!context.ledgerMaster.haveLedger (seq))
        return false;

    if (seq > context.ledgerMaster.getValidatedLedger ()->info().seq)
        return false;

    return context.ledgerMaster.getHashBySeq (seq) == hash;
}

bool
getMetaHex (Ledger const& ledger,
    uint256 const& transID, std::string& hex)
{
    SHAMapTreeNode::TNType type;
    auto const item =
        ledger.txMap().peekItem (transID, type);

    if (!item)
        return false;

    if (type != SHAMapTreeNode::tnTRANSACTION_MD)
        return false;

    SerialIter it (item->slice());
    it.getVL (); // skip transaction
    hex = strHex (makeSlice(it.getVL ()));
    return true;
}

Json::Value doTx (RPC::Context& context)
{
    if (!context.params.isMember (jss::transaction))
        return rpcError (rpcINVALID_PARAMS);

    bool binary = context.params.isMember (jss::binary)
            && context.params[jss::binary].asBool ();

    auto const txid  = context.params[jss::transaction].asString ();

    if (!isHexTxID (txid))
        return rpcError (rpcNOT_IMPL);

    auto txn = context.app.getMasterTransaction ().fetch (
        from_hex_text<uint256>(txid), true);

    if (!txn)
        return rpcError (rpcTXN_NOT_FOUND);

    Json::Value ret = txn->getJson (1, binary);

    if (txn->getLedger () == 0)
        return ret;

    if (auto lgr = context.ledgerMaster.getLedgerBySeq (txn->getLedger ()))
    {
        bool okay = false;

        if (binary)
        {
            std::string meta;

            if (getMetaHex (*lgr, txn->getID (), meta))
            {
                ret[jss::meta] = meta;
                okay = true;
            }
        }
        else
        {
            auto rawMeta = lgr->txRead (txn->getID()).second;
            if (rawMeta)
            {
                auto txMeta = std::make_shared<TxMeta> (txn->getID (),
                    lgr->seq (), *rawMeta, context.app.journal ("TxMeta"));
                okay = true;
                auto meta = txMeta->getJson (0);
                addPaymentDeliveredAmount (meta, context, txn, txMeta);
                ret[jss::meta] = meta;
            }
        }

	// add ledger_current_index field to result 
	ret[jss::ledger_current_index] =
		context.ledgerMaster.getCurrentLedgerIndex();

        if (okay)
            ret[jss::validated] = isValidated (
                context, lgr->info().seq, lgr->info().hash);
    }

    return ret;
}

#ifdef IPFS_ENABLE
extern boost::optional<Json::Value> parseExtraInfo(STTx const& tx, beast::Journal const& j);
extern bool checkFilesInfo(STTx const& tx, Json::Value const& filesInfo, bool local, bool upload,
                           std::string &filePath, Application &app);

class httpstream : public std::iostream
{
    class httpbuf : public std::streambuf
    {
        Json::Output &output_;
        Json::Value &file_;
        std::uint64_t bytes_;
        std::uint64_t size_;
        httpstream *owner_;
        bool reply_;

    public:
        httpbuf (Json::Output &output, Json::Value &file) : output_ (output), file_ (file)
        {
            bytes_ = 0;
            size_ = 0;
            reply_ = false;
        }

        bool hasReply()
        {
            return reply_;
        }

        bool isComplete()
        {
            return reply_ && bytes_ >= size_;
        }

        void setOwner(httpstream *owner)
        {
            owner_ = owner;
        }
    protected:
        int overflow (int c) override
        {
            auto ch = (char)c;
            return xsputn (&ch, 1) == 1 ? c : EOF;
        }

        std::streamsize xsputn (const char* s, std::streamsize n) override
        {
            if (!reply_)
            {
                if (!owner_->good())
                {
                    owner_->rdbuf(&owner_->error_);
                    return owner_->error_.sputn(s, n);
                }

                size_ = file_[jss::size].asUInt();

                output_ ("HTTP/1.1 200 OK\r\n");
                output_ (getHTTPHeaderTimestamp ());
                output_ ("Connection: Keep-Alive\r\n"
                         "Content-Length: ");
                output_ (std::to_string(size_));

                if (file_.isMember(jss::content_type))
                {
                    output_ ("\r\n"
                             "Content-Type: " + file_[jss::content_type].asString() + "\r\n");
                }
                else
                {
                    output_ ("\r\n"
                             "Content-Type: application/octet-stream\r\n");
                }

                output_ ("Server: " + systemName () + "-json-rpc/");
                output_ (BuildInfo::getFullVersionString ());
                output_ ("\r\n"
                         "\r\n");

                reply_ = true;
            }

            if (bytes_ < size_)
            {
                if (bytes_ + n > size_)
                {
                    n = size_ - bytes_;
                }
                output_ (boost::string_ref(s, n));
                bytes_ += n;
                return n;
            }

            return 0;
        }
    };

    httpbuf buf_;
    std::stringbuf error_;
public:
    httpstream (Json::Output &output, Json::Value &file) : std::iostream (&buf_),
        buf_ (output, file)
    {
        buf_.setOwner(this);
    }

    bool isComplete ()
    {
        return buf_.isComplete ();
    }

    bool hasReply()
    {
        return buf_.hasReply();
    }
};

#endif

Json::Value doFileDownload(RPC::Context &context)
{
#ifdef IPFS_ENABLE
    Json::Value ret = Json::objectValue;

    if (!context.params.isMember (jss::transaction))
        return rpcError (rpcINVALID_PARAMS);

    auto const txid  = context.params[jss::transaction].asString ();
    if (!isHexTxID (txid))
        return rpcError (rpcINVALID_PARAMS);

    auto txn = context.app.getMasterTransaction ().fetch (
        from_hex_text<uint256>(txid), true);
    if (!txn)
        return rpcError (rpcTXN_NOT_FOUND);

    auto &tx = *txn->getSTransaction();
    if (tx.getTxnType() != ttPAYMENT)
        return rpcError (rpcINVALID_PARAMS);

    if (!context.params.isMember (jss::password))
        return rpcError (rpcINVALID_PARAMS);

    auto passwd = context.params[jss::password].asString();
    if (!isHex64(passwd))
        return rpcError (rpcINVALID_PARAMS);

    std::string myPasswd;
    if (tx.addon.isMember(jss::password))
    {
        myPasswd = tx.addon[jss::password].asString();
    }
    else
    {
        myPasswd = to_string(rand_uint256());
        tx.addon[jss::password] = myPasswd;
    }

    if (myPasswd != passwd)
    {
        ret[jss::NewPassword] = myPasswd;
        return rpcError (rpcPASSWD_CHANGED, ret);
    }

    // check identity consistent with tx
    //if (context.role == Role::ADMIN)
    auto keypair = RPC::keypairForSignature(context.params, ret);
    auto publicKey = PublicKey(makeSlice(tx.getFieldVL(sfSigningPubKey)));
    if (RPC::contains_error(ret))
    {
        if (!context.params.isMember (jss::signature))
            return rpcError (rpcINVALID_PARAMS);

        auto sign = strUnHex(context.params[jss::signature].asString());
        if (!sign.second)
            return rpcError (rpcINVALID_PARAMS);

        auto m = txid + passwd;
        if (!verify (publicKey, makeSlice(m), makeSlice(sign.first), false))
        {
            return rpcError (rpcFORBIDDEN);
        }
    }
    else if (keypair.first != publicKey)
    {
        return rpcError (rpcBAD_SECRET);
    }

    std::uint32_t expiration = 0;
    if (txn->getLedger() == 0)
    {
        if (txn->getResult() != tesSUCCESS)
        {
            return rpcError (rpcNO_PERMISSION);
        }
    }
    else
    {
        auto lgr = context.ledgerMaster.getLedgerBySeq (txn->getLedger ());
        if (!lgr)
        {
            return rpcError (rpcLGR_NOT_FOUND);
        }

        if (!isValidated (context, lgr->info().seq, lgr->info().hash))
        {
            return rpcError (rpcLGR_NOT_VALIDATED);
        }

        auto rawMeta = lgr->txRead (txn->getID()).second;
        if (!rawMeta)
        {
            return rpcError (rpcTXN_NOT_FOUND);
        }

        auto txMeta = std::make_shared<TxMeta> (txn->getID (),
                                                lgr->seq (), *rawMeta,
                                                context.app.journal ("TxMeta"));
        if (!txMeta || txMeta->getResult() != tesSUCCESS)
        {
            return rpcError (rpcINTERNAL);
        }

        tx.meta = txMeta->getMetaExtra();
    }

    expiration = tx.meta.getFieldU32(sfExpiration);
    auto now = context.app.timeKeeper().now().time_since_epoch().count();
    if (now > expiration)
    {
        return rpcError (rpcNO_PERMISSION);
    }

    auto extra = parseExtraInfo (tx, context.j);
    if (!extra)
    {
        return rpcError (rpcNO_PERMISSION);
    }

    auto filesInfo = *extra;
    auto fileOpType =  filesInfo[jss::type];
    if (fileOpType != jss::FileDownload)
    {
        return rpcError (rpcNO_PERMISSION);
    }

    std::string filePath;
    if (!checkFilesInfo (tx, filesInfo, false, false, filePath, context.app))
    {
        return rpcError (rpcNO_PERMISSION);
    }

    tx.addon.removeMember (jss::password);
    std::string message;
    auto &file = filesInfo[jss::files][0U];
    auto fileId = file[jss::id].asString();
    httpstream response(*context.output, file);
    try {
        auto client = context.app.createIpfsClient();
        //file[jss::content_type] = "application/json;; charset=UTF-8";
        client->FilesGet (fileId, &response);

        /*
        ret[jss::result] = "Download file " + fileId + " OK by httpstream!";
        auto content = to_string(ret);
        file[jss::size] = (Json::Value::UInt)content.size();
        file[jss::content_type] = "application/json; charset=UTF-8";
        response.write(content.c_str(), content.size());
        */
        //HTTPReply (200, to_string(ret), *context.output, context.j);
    } catch (std::exception &e) {
        if (!response.isComplete())
        {
            message = (const char *)jss::FileDownload + (" failed: " + fileId);
            JLOG(context.j.error()) << message << ", reason = " << e.what();
        }
    }

    if (response.hasReply())
    {
        context.noReply = true; // telll rpc framework not to send http reply
        context.closeSess = !response.isComplete();
    }
    else
    {
        ret = RPC::make_error(rpcINTERNAL, message);
    }

    return ret;
#else
    return rpcError (rpcNOT_SUPPORTED);
#endif
}

} //
