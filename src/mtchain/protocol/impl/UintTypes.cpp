//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/Serializer.h>
#include <mtchain/protocol/SystemParameters.h>
#include <mtchain/protocol/UintTypes.h>
#include <mtchain/protocol/types.h>

namespace mtchain {

static const int CURRENCY_CODE_LENGTH = 3;
static const int CURRENCY_CODE_LENGTH_MAX = 5;
static const int CURRENCY_CODE_LENGTH_MIN = 2;

std::string to_string(Currency const& currency)
{
    // Characters we are willing to allow in the ASCII representation of a
    // three-letter currency code.
    static std::string const allowed_characters =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "<>(){}[]|?!@#$%^&*";

    if (currency == zero)
        return systemCurrencyCode();

    if (currency == noCurrency())
        return "1";

    static Currency const sIsoBits (
        from_hex_text<Currency>("FFFFFFFFFFFFFFFFFFFF0000000000FFFFFFFFFF"));

    if ((currency & sIsoBits).isZero ())
    {
        // The offset of the 3 character ISO code in the currency descriptor
        int const isoOffset = 12 - 2; //CURRENCY_CODE_LENGTH_MAX = 5 - CURRENCY_CODE_LENGTH = 3;

        std::string iso(
            currency.data () + isoOffset,
            currency.data () + isoOffset + CURRENCY_CODE_LENGTH_MAX);

		if (!iso.empty())
		{
			iso.erase(0, iso.find_first_not_of('\0'));
			//s.erase(s.find_last_not_of(" ") + 1);
		}

        // Specifying the system currency code using ISO-style representation
        // is not allowed.
        if ((iso != systemCurrencyCode()) &&
            (iso.find_first_not_of (allowed_characters) == std::string::npos))
        {
            return iso;
        }
    }

    return strHex (currency.begin (), currency.size ());
}

bool to_currency(Currency& currency, std::string const& code)
{
    if (code.empty () || !code.compare (systemCurrencyCode()))
    {
        currency = zero;
        return true;
    }

    if (code.size () >= CURRENCY_CODE_LENGTH_MIN && code.size() <= CURRENCY_CODE_LENGTH_MAX)
    {
        Blob codeBlob (code.size());

        std::transform (code.begin (), code.end (), codeBlob.begin (), ::toupper);

        Serializer  s;

        //s.addZeros (96 / 8);
		s.addZeros(96 / 8 - (code.size() - CURRENCY_CODE_LENGTH));
        s.addRaw (codeBlob);
        s.addZeros (16 / 8);
        s.addZeros (24 / 8);

        s.get160 (currency, 0);
        return true;
    }

    if (40 == code.size ())
        return currency.SetHex (code);

    return false;
}

Currency to_currency(std::string const& code)
{
    Currency currency;
    if (!to_currency(currency, code))
        currency = noCurrency();
    return currency;
}

Currency const& mCurrency()
{
    static Currency const currency(0);
    return currency;
}

Currency const& noCurrency()
{
    static Currency const currency(1);
    return currency;
}

Currency const& badCurrency()
{
    static Currency const currency(0x5852500000000000);
    return currency;
}

} //
