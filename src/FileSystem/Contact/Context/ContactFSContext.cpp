#include "ContactFSContext.h"

ContactFSContext::ContactFSContext(ContactFSStrategy* s)
{
    this->strategy = s;
}

QList<ContactItem> ContactFSContext::read(QString file)
{
    return this->strategy->read(file);
}

void ContactFSContext::write(QString file, QString data)
{
    this->strategy->write(file, data);
}