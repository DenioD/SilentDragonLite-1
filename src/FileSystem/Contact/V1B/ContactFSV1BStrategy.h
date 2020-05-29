#ifndef CONTACTFSV1BSTRATEGY_H
#define CONTACTFSV1BSTRATEGY_H

#include "../ContactFSStrategy.h"

class ContactFSV1BStrategy : public ContactFSStrategy
{
    virtual QList<ContactItem> read(QString file);
    virtual void write(QString file, QString data);
};

#endif