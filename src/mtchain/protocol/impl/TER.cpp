//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/TER.h>
#include <unordered_map>
#include <type_traits>

namespace mtchain {

bool transResultInfo (TER code, std::string& token, std::string& text)
{
    static
    std::unordered_map<
        std::underlying_type_t<TER>,
        std::pair<char const* const, char const* const>> const
    results
    {
        { tecCLAIM,                  { "tecCLAIM",                 "Fee claimed. Sequence used. No action."                                        } },
        { tecDIR_FULL,               { "tecDIR_FULL",              "Can not add entry to full directory."                                          } },
        { tecFAILED_PROCESSING,      { "tecFAILED_PROCESSING",     "Failed to correctly process transaction."                                      } },
        { tecINSUF_RESERVE_LINE,     { "tecINSUF_RESERVE_LINE",    "Insufficient reserve to add trust line."                                       } },
        { tecINSUF_RESERVE_OFFER,    { "tecINSUF_RESERVE_OFFER",   "Insufficient reserve to create offer."                                         } },
        { tecNO_DST,                 { "tecNO_DST",                "Destination does not exist. Send FPA to create it."                            } },
        { tecNO_DST_INSUF_M,         { "tecNO_DST_INSUF_FPA",      "Destination does not exist. Too little FPA sent to create it."                 } },
        { tecNO_LINE_INSUF_RESERVE,  { "tecNO_LINE_INSUF_RESERVE", "No such line. Too little reserve to create it."                                } },
        { tecNO_LINE_REDUNDANT,      { "tecNO_LINE_REDUNDANT",     "Can't set non-existent line to default."                                       } },
        { tecPATH_DRY,               { "tecPATH_DRY",              "Path could not send partial amount."                                           } },
        { tecPATH_PARTIAL,           { "tecPATH_PARTIAL",          "Path could not send full amount."                                              } },
        { tecNO_ALTERNATIVE_KEY,     { "tecNO_ALTERNATIVE_KEY",    "The operation would remove the ability to sign transactions with the account." } },
        { tecNO_REGULAR_KEY,         { "tecNO_REGULAR_KEY",        "Regular key is not set."                                                       } },
        { tecOVERSIZE,               { "tecOVERSIZE",              "Object exceeded serialization limits."                                         } },
        { tecUNFUNDED,               { "tecUNFUNDED",              "One of _ADD, _OFFER, or _SEND. Deprecated."                                    } },
        { tecUNFUNDED_ADD,           { "tecUNFUNDED_ADD",          "Insufficient FPA balance for WalletAdd."                                       } },
        { tecUNFUNDED_OFFER,         { "tecUNFUNDED_OFFER",        "Insufficient balance to fund created offer."                                   } },
        { tecUNFUNDED_PAYMENT,       { "tecUNFUNDED_PAYMENT",      "Insufficient FPA balance to send."                                             } },
        { tecOWNERS,                 { "tecOWNERS",                "Non-zero owner count."                                                         } },
        { tecNO_ISSUER,              { "tecNO_ISSUER",             "Issuer account does not exist."                                                } },
        { tecNO_AUTH,                { "tecNO_AUTH",               "Not authorized to hold asset."                                                 } },
        { tecNO_LINE,                { "tecNO_LINE",               "No such line."                                                                 } },
        { tecINSUFF_FEE,             { "tecINSUFF_FEE",            "Insufficient balance to pay fee."                                              } },
        { tecFROZEN,                 { "tecFROZEN",                "Asset is frozen."                                                              } },
        { tecNO_TARGET,              { "tecNO_TARGET",             "Target account does not exist."                                                } },
        { tecNO_PERMISSION,          { "tecNO_PERMISSION",         "No permission to perform requested operation."                                 } },
        { tecNO_ENTRY,               { "tecNO_ENTRY",              "No matching entry found."                                                      } },
        { tecINSUFFICIENT_RESERVE,   { "tecINSUFFICIENT_RESERVE",  "Insufficient reserve to complete requested operation."                         } },
        { tecNEED_MASTER_KEY,        { "tecNEED_MASTER_KEY",       "The operation requires the use of the Master Key."                             } },
        { tecDST_TAG_NEEDED,         { "tecDST_TAG_NEEDED",        "A destination tag is required."                                                } },
        { tecINTERNAL,               { "tecINTERNAL",              "An internal error has occurred during processing."                             } },
        { tecCRYPTOCONDITION_ERROR,  { "tecCRYPTOCONDITION_ERROR", "Malformed, invalid, or mismatched conditional or fulfillment."                 } },

        { tefALREADY,                { "tefALREADY",               "The exact transaction was already in this ledger."                             } },
        { tefBAD_ADD_AUTH,           { "tefBAD_ADD_AUTH",          "Not authorized to add account."                                                } },
        { tefBAD_AUTH,               { "tefBAD_AUTH",              "Transaction's public key is not authorized."                                   } },
        { tefBAD_LEDGER,             { "tefBAD_LEDGER",            "Ledger in unexpected state."                                                   } },
        { tefBAD_QUORUM,             { "tefBAD_QUORUM",            "Signatures provided do not meet the quorum."                                   } },
        { tefBAD_SIGNATURE,          { "tefBAD_SIGNATURE",         "A signature is provided for a non-signer."                                     } },
        { tefCREATED,                { "tefCREATED",               "Can't add an already created account."                                         } },
        { tefEXCEPTION,              { "tefEXCEPTION",             "Unexpected program state."                                                     } },
        { tefFAILURE,                { "tefFAILURE",               "Failed to apply."                                                              } },
        { tefINTERNAL,               { "tefINTERNAL",              "Internal error."                                                               } },
        { tefMASTER_DISABLED,        { "tefMASTER_DISABLED",       "Master key is disabled."                                                       } },
        { tefMAX_LEDGER,             { "tefMAX_LEDGER",            "Ledger sequence too high."                                                     } },
        { tefNO_AUTH_REQUIRED,       { "tefNO_AUTH_REQUIRED",      "Auth is not required."                                                         } },
        { tefNOT_MULTI_SIGNING,      { "tefNOT_MULTI_SIGNING",     "Account has no appropriate list of multi-signers."                             } },
        { tefPAST_SEQ,               { "tefPAST_SEQ",              "This sequence number has already past."                                        } },
        { tefWRONG_PRIOR,            { "tefWRONG_PRIOR",           "This previous transaction does not match."                                     } },
        { tefBAD_AUTH_MASTER,        { "tefBAD_AUTH_MASTER",       "Auth for unclaimed account needs correct master key."                          } },
        { tefASSET_EXIST,            { "tefASSET_EXIST",           "The same asset has exists!"                                                    } },
        { tefNO_OWNER,               { "tefNO_OWNER",              "The specified owner account doesn't exist!"                                    } },
        { tefNO_ASSET,               { "tefNO_ASSET",              "The specified asset doesn't exist!"                                            } },
        { tefTOKEN_EXIST,            { "tefTOKEN_EXIST",           "The same token has exists!"                                                    } },
        { tefNO_TOKEN,               { "tefNO_TOKEN",              "The specified token doesn't exist!"                                            } },
        { tefDST_IS_OWNER,           { "tefDST_IS_OWNER",          "The destination has been the owner of the specified token!"                    } },
        { tefDESTROY_ASSET,          { "tefDESTROY_ASSET",         "The specified asset can't be destroyed!"                                       } },


        { telLOCAL_ERROR,            { "telLOCAL_ERROR",           "Local failure."                                                                } },
        { telBAD_DOMAIN,             { "telBAD_DOMAIN",            "Domain too long."                                                              } },
        { telBAD_PATH_COUNT,         { "telBAD_PATH_COUNT",        "Malformed: Too many paths."                                                    } },
        { telBAD_PUBLIC_KEY,         { "telBAD_PUBLIC_KEY",        "Public key too long."                                                          } },
        { telFAILED_PROCESSING,      { "telFAILED_PROCESSING",     "Failed to correctly process transaction."                                      } },
        { telINSUF_FEE_P,            { "telINSUF_FEE_P",           "Fee insufficient."                                                             } },
        { telNO_DST_PARTIAL,         { "telNO_DST_PARTIAL",        "Partial payment to create account not allowed."                                } },
        { telCAN_NOT_QUEUE,          { "telCAN_NOT_QUEUE",         "Can not queue at this time." } },

        { temMALFORMED,              { "temMALFORMED",             "Malformed transaction."                                                        } },
        { temBAD_AMOUNT,             { "temBAD_AMOUNT",            "Can only send positive amounts."                                               } },
        { temBAD_CURRENCY,           { "temBAD_CURRENCY",          "Malformed: Bad currency."                                                      } },
        { temBAD_EXPIRATION,         { "temBAD_EXPIRATION",        "Malformed: Bad expiration."                                                    } },
        { temBAD_FEE,                { "temBAD_FEE",               "Invalid fee, negative or not FPA."                                             } },
        { temBAD_ISSUER,             { "temBAD_ISSUER",            "Malformed: Bad issuer."                                                        } },
        { temBAD_LIMIT,              { "temBAD_LIMIT",             "Limits must be non-negative."                                                  } },
        { temBAD_OFFER,              { "temBAD_OFFER",             "Malformed: Bad offer."                                                         } },
        { temBAD_PATH,               { "temBAD_PATH",              "Malformed: Bad path."                                                          } },
        { temBAD_PATH_LOOP,          { "temBAD_PATH_LOOP",         "Malformed: Loop in path."                                                      } },
        { temBAD_QUORUM,             { "temBAD_QUORUM",            "Malformed: Quorum is unreachable."                                             } },
        { temBAD_SEND_M_LIMIT,       { "temBAD_SEND_FPA_LIMIT",    "Malformed: Limit quality is not allowed for FPA to FPA."                       } },
        { temBAD_SEND_M_MAX,         { "temBAD_SEND_FPA_MAX",      "Malformed: Send max is not allowed for FPA to FPA."                            } },
        { temBAD_SEND_M_NO_DIRECT,   { "temBAD_SEND_FPA_NO_DIRECT","Malformed: No FinPal direct is not allowed for FPA to FPA."                    } },
        { temBAD_SEND_M_PARTIAL,     { "temBAD_SEND_FPA_PARTIAL",  "Malformed: Partial payment is not allowed for FPA to FPA."                     } },
        { temBAD_SEND_M_PATHS,       { "temBAD_SEND_FPA_PATHS",    "Malformed: Paths are not allowed for FPA to FPA."                              } },
        { temBAD_SEQUENCE,           { "temBAD_SEQUENCE",          "Malformed: Sequence is not in the past."                                       } },
        { temBAD_SIGNATURE,          { "temBAD_SIGNATURE",         "Malformed: Bad signature."                                                     } },
        { temBAD_SIGNER,             { "temBAD_SIGNER",            "Malformed: No signer may duplicate account or other signers."                  } },
        { temBAD_SRC_ACCOUNT,        { "temBAD_SRC_ACCOUNT",       "Malformed: Bad source account."                                                } },
        { temBAD_TRANSFER_RATE,      { "temBAD_TRANSFER_RATE",     "Malformed: Transfer rate must be >= 1.0"                                       } },
        { temBAD_WEIGHT,             { "temBAD_WEIGHT",            "Malformed: Weight must be a positive value."                                   } },
        { temDST_IS_SRC,             { "temDST_IS_SRC",            "Destination may not be source."                                                } },
        { temDST_NEEDED,             { "temDST_NEEDED",            "Destination not specified."                                                    } },
        { temINVALID,                { "temINVALID",               "The transaction is ill-formed."                                                } },
        { temINVALID_FLAG,           { "temINVALID_FLAG",          "The transaction has an invalid flag."                                          } },
        { temREDUNDANT,              { "temREDUNDANT",             "Sends same currency to self."                                                  } },
        { temMTCHAIN_EMPTY,          { "temFinPal_EMPTY",          "PathSet with no paths."                                                        } },
        { temUNCERTAIN,              { "temUNCERTAIN",             "In process of determining result. Never returned."                             } },
        { temUNKNOWN,                { "temUNKNOWN",               "The transaction requires logic that is not implemented yet."                   } },
        { temDISABLED,               { "temDISABLED",              "The transaction requires logic that is currently disabled."                    } },
        { temBAD_TICK_SIZE,          { "temBAD_TICK_SIZE",         "Malformed: Tick size out of range."                                            } },
        { temBAD_ASSET,              { "temBAD_ASSET",             "Malformed: Bad Asset."                                                         } },
        { temBAD_TOKEN,              { "temBAD_TOKEN",             "Malformed: Bad Token."                                                         } },

        { terRETRY,                  { "terRETRY",                 "Retry transaction."                                                            } },
        { terFUNDS_SPENT,            { "terFUNDS_SPENT",           "Can't set password, password set funds already spent."                         } },
        { terINSUF_FEE_B,            { "terINSUF_FEE_B",           "Account balance can't pay fee."                                                } },
        { terLAST,                   { "terLAST",                  "Process last."                                                                 } },
        { terNO_MTCHAIN,             { "terNO_FinPal",             "Path does not permit rippling."                                                } },
        { terNO_ACCOUNT,             { "terNO_ACCOUNT",            "The source account does not exist."                                            } },
        { terNO_AUTH,                { "terNO_AUTH",               "Not authorized to hold IOUs."                                                  } },
        { terNO_LINE,                { "terNO_LINE",               "No such line."                                                                 } },
        { terPRE_SEQ,                { "terPRE_SEQ",               "Missing/inapplicable prior transaction."                                       } },
        { terOWNERS,                 { "terOWNERS",                "Non-zero owner count."                                                         } },
        { terQUEUED,                 { "terQUEUED",                "Held until escalated fee drops."                                               } },

        { tesSUCCESS,                { "tesSUCCESS",               "The transaction was applied. Only final in a validated ledger."                } },
        { tecOVERISSUE,              { "tecOVERISSUE",             "Issue too much over limit."                                                    } },
        { tecUNFUNDED_DEAL_OFFER,    { "tecUNFUNDED_DEAL_OFFER",   "Cann't deal the offer."                                                        } },
        { tecFAILED_IOU_FEE,         { "tecFAILED_IOU_FEE",        "Failed to pay iou fee."                                                        } },
        { tecNO_LINE_IOU_FEE,        { "tecNO_LINE_IOU_FEE",       "No line to pay iou fee."                                                       } },
        { tecEXCEED_TRUST_LIMIT,     { "tecEXCEED_TRUST_LIMIT",    "Exceed iou limit which users permit."                                          } },
        { tecUNFUNDED_PAYMENT_IOU,   { "tecUNFUNDED_PAYMENT_IOU",  "Insufficient IOU balance to send."                                             } },
        { tecUNFUNDED_PAYMENT_IOU_FEE, { "tecUNFUNDED_PAYMENT_IOU_FEE",  "Insufficient balance to pay iou fee."                                    } },
        { tecIOU_PRECISION_MISMATCH, { "tecIOU_PRECISION_MISMATCH","IOU decimal is too small."                                                     } },
        { tecFORBID_TRUST_LINE,      { "tecFORBID_TRUST_LINE",     "Issuer must use 'AccountSet' to set flag 8 at first!"                          } },
    };

    auto const r = results.find (
        static_cast<std::underlying_type_t<TER>> (code));

    if (r == results.end())
        return false;

    token = r->second.first;
    text = r->second.second;
    return true;
}

std::string transToken (TER code)
{
    std::string token;
    std::string text;

    return transResultInfo (code, token, text) ? token : "-";
}

std::string transHuman (TER code)
{
    std::string token;
    std::string text;

    return transResultInfo (code, token, text) ? text : "-";
}

} //
