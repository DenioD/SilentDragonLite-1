#ifndef CONTACTFSV2STRATEGY_H
#define CONTACTFSV2STRATEGY_H

#include "../ContactFSStrategy.h"

class ContactFSV2Strategy : public ContactFSStrategy
{
    virtual QList<ContactItem> read(QString file);
    virtual void write(QString file, QString data);
};

#endif