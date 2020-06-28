#include "camount.h"
#include "settings.h"
#include "precompiled.h"

const int NUMPLACES = 8;
const qint64 COIN = 100000000;

double CAmount::toDecimalDouble() const 
{
    return static_cast<double>(this->amount) / COIN;
}

QString CAmount::toDecimalString() const 
{
    if (amount < 0) 
    {
        CAmount negative(-1 * this->amount);
        return "-" + negative.toDecimalString();
    }

    int wholePart = amount / COIN;
    int decimalPart = amount % COIN;
    QString r = QString::number(wholePart);
    if (decimalPart > 0)
    {
        QString decimalPartStr = QString::number(decimalPart);
        r = r + "." + decimalPartStr.rightJustified(NUMPLACES, '0');
        // Trim tailing 0s
        while (r.right(1) == "0")
            r = r.left(r.length() - 1);

    }

    return r;
}

double CAmount::getDblAmount() const
{
    return static_cast<double>(this->amount) / COIN;
}

QString CAmount::toDecimalUSDString() const 
{
    double price = Settings::getInstance()->getZECPrice();
    return "$ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalEURString() const 
{
    double price = Settings::getInstance()->getEURPrice();
    return QLocale(QLocale::German).toString(this->getDblAmount() * price, 'f', 2) + " €";
}

QString CAmount::toDecimalBTCString() const 
{
    double price = Settings::getInstance()->getBTCPrice();
    return "BTC " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 9);
}

QString CAmount::toDecimalCNYString() const 
{
    double price = Settings::getInstance()->getCNYPrice();
    return "¥ /元  " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}
QString CAmount::toDecimalRUBString() const 
{
    double price = Settings::getInstance()->getRUBPrice();
    return "₽ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalCADString() const 
{
    double price = Settings::getInstance()->getCADPrice();
    return "$ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalSGDString() const 
{
    double price = Settings::getInstance()->getSGDPrice();
    return "$ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalCHFString() const 
{
    double price = Settings::getInstance()->getCHFPrice();
    return "CHF " + QLocale(QLocale::German).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalINRString() const 
{
    double price = Settings::getInstance()->getINRPrice();
    return "₹ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}
QString CAmount::toDecimalGBPString() const 
{
    double price = Settings::getInstance()->getGBPPrice();
    return "£ " + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}
QString CAmount::toDecimalAUDString() const 
{
    double price = Settings::getInstance()->getAUDPrice();
    return " $" + QLocale(QLocale::English).toString(this->getDblAmount() * price, 'f', 2);
}

QString CAmount::toDecimalhushString() const 
{
    return this->toDecimalString() % " " % Settings::getTokenName();
}

QString CAmount::toDecimalhushUSDString() const 
{
    auto usdString = this->toDecimalUSDString();
    if (!usdString.isEmpty())
        return this->toDecimalhushString() % " (" % usdString % ")";
        
    else
        return this->toDecimalhushString();
}


QString CAmount::toDecimalhushEURString() const 
{
    auto eurString = this->toDecimalEURString();
    if (!eurString.isEmpty())
        return this->toDecimalhushString() % " (" % eurString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushBTCString() const 
{
    auto btcString = this->toDecimalBTCString();
    if (!btcString.isEmpty())
        return this->toDecimalhushString() % " (" % btcString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushCNYString() const 
{
    auto cnyString = this->toDecimalCNYString();
    if (!cnyString.isEmpty())
        return this->toDecimalhushString() % " (" % cnyString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushRUBString() const 
{
    auto rubString = this->toDecimalRUBString();
    if (!rubString.isEmpty())
        return this->toDecimalhushString() % " (" % rubString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushCADString() const 
{
    auto cadString = this->toDecimalCADString();
    if (!cadString.isEmpty())
        return this->toDecimalhushString() % " (" % cadString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushSGDString() const 
{
    auto sgdString = this->toDecimalSGDString();
    if (!sgdString.isEmpty())
        return this->toDecimalhushString() % " (" % sgdString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushCHFString() const 
{
    auto chfString = this->toDecimalCHFString();
    if (!chfString.isEmpty())
        return this->toDecimalhushString() % " (" % chfString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushINRString() const 
{
    auto inrString = this->toDecimalINRString();
    if (!inrString.isEmpty())
        return this->toDecimalhushString() % " (" % inrString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushGBPString() const 
{
    auto gbpString = this->toDecimalGBPString();
    if (!gbpString.isEmpty())
        return this->toDecimalhushString() % " (" % gbpString % ")";

    else
        return this->toDecimalhushString();
}

QString CAmount::toDecimalhushAUDString() const 
{
    auto audString = this->toDecimalAUDString();
    if (!audString.isEmpty())
        return this->toDecimalhushString() % " (" % audString % ")";

    else
        return this->toDecimalhushString();
}


CAmount CAmount::fromDecimalString(QString decimalString) 
{
    auto amtParts = decimalString.split(".");
    qint64 r = amtParts[0].toULongLong() * COIN;
    if (amtParts.length() == 2) 
    {
        auto trailingZeros = QString("0").repeated(NUMPLACES - amtParts[1].length());
        r += QString(amtParts[1] + trailingZeros).toULongLong();
    }

    return CAmount(r);
}