#ifndef CONTACTFSV1ASTRATEGY_H
#define CONTACTFSV1ASTRATEGY_H

#include "../ContactFSStrategy.h"

class ContactFSV1AStrategy : public ContactFSStrategy
{
    virtual QList<ContactItem> read(QString file);
    virtual void write(QString file, QString data);
};

#endif