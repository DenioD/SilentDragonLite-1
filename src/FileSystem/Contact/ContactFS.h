#ifndef CONTACTFS_H
#define CONTACTFS_H
#include <QList>
#include "./Context/ContactFSContext.h"
#include "ContactFSStrategy.h"
#include "./V1A/ContactFSV1AStrategy.h"
#include "./V1B/ContactFSV1BStrategy.h"
#include "./V2/ContactFSV2Strategy.h"
#include "./V3/ContactFSV3Strategy.h"

class ContactFS
{
    public:
        static QList<ContactItem> readContacts(QString file);
        static void write(QString file, QString data);
};

#endif