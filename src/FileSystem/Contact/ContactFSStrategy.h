#ifndef CONTACTFSSTRATEGY_H
#define CONTACTFSSTRATEGY_H

#include <QList>
#include "../../Model/ContactItem.h"

class ContactFSStrategy
{
    public:
        virtual QList<ContactItem> read(QString file) {};
        virtual void write(QString file, QString data) {};
};

#endif