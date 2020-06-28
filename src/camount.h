#ifndef CAMOUNT_H
#define CAMOUNT_H

#include "precompiled.h"

class CAmount {
private:
    CAmount(qint64 amount) {
        this->amount = amount;
    }

    qint64 amount;

public:
    static CAmount fromDecimalString(QString decimalString);
    static CAmount fromqint64(qint64 a) {
        return CAmount(a);
    }
    static CAmount fromDouble(double d) {
        return CAmount::fromDecimalString(QString::number(d, 'f', 8));
    }

    CAmount() : amount(0) {};
    CAmount(const CAmount&) = default;
    ~CAmount() = default;

    double  toDecimalDouble() const;
    QString toDecimalString() const;
    double getDblAmount() const;
    QString toDecimalUSDString() const;
    QString toDecimalEURString() const;
    QString toDecimalBTCString() const;
    QString toDecimalCNYString() const;
    QString toDecimalRUBString() const;
    QString toDecimalCADString() const;
    QString toDecimalSGDString() const;
    QString toDecimalCHFString() const;
    QString toDecimalINRString() const;
    QString toDecimalGBPString() const;
    QString toDecimalAUDString() const;
    QString toDecimalhushString() const;
    QString toDecimalhushUSDString() const;
    QString toDecimalhushEURString() const;
    QString toDecimalhushBTCString() const;
    QString toDecimalhushCNYString() const;
    QString toDecimalhushRUBString() const;
    QString toDecimalhushCADString() const;
    QString toDecimalhushSGDString() const;
    QString toDecimalhushCHFString() const;
    QString toDecimalhushINRString() const;
    QString toDecimalhushGBPString() const;
    QString toDecimalhushAUDString() const;
    
    qint64  toqint64() const { return amount; };

    CAmount operator+ (const CAmount& other) const {
        return CAmount(this->amount + other.amount);
    }  

    CAmount operator- (const CAmount& other) const {
        return CAmount(this->amount - other.amount);
    }

    bool operator< (const CAmount& other) const {
        return this->amount < other.amount;
    }

    bool operator< (const qint64& other) const {
        return this->amount < other;
    }

    bool operator> (const CAmount& other) const {
        return this->amount > other.amount;
    }
    bool operator== (const qint64 other) const {
        return this->amount == other;
    }
};

#endif // CAMOUNT_H
