#ifndef CONTACTFSCONTEXT_H
#define CONTACTFSCONTEXT_H

#include "../ContactFSStrategy.h"

class ContactFSContext
{
    private:
        ContactFSStrategy* strategy = nullptr;

    public:
        ContactFSContext(ContactFSStrategy* s);
        QList<ContactItem> read(QString file);
        void write(QString file, QString data);
};

#endif