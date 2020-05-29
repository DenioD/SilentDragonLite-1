#ifndef CONTACTFSV3STRATEGY_H
#define CONTACTFSV3STRATEGY_H

#include "../ContactFSStrategy.h"

class ContactFSV3Strategy : public ContactFSStrategy
{
    virtual QList<ContactItem> read(QString file);
    virtual void write(QString file, QString data);
};

#endif