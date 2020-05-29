#include "ContactFSV3Strategy.h"


QList<ContactItem> ContactFSV3Strategy::read(QString file)
{
    qDebug() << "Reading v1a addresses";
    QFile _file(file);
    _file.open(QIODevice::ReadOnly);
    QDataStream in(&_file);
    QList<ContactItem> contacts;
    QString version;

    in >> version;

    QList<QPair<QString,QString>> stuff;
    in >> stuff;

    qDebug() << "Stuff: " << stuff;
    for (int i=0; i < stuff.size(); i++) 
    {
        ContactItem contact = ContactItem(
            stuff[i].first, 
            stuff[i].second
        );
        contacts.push_back(contact);
        qDebug() << "Contact: " << contact.toQTString();
    }

    _file.close();
    return contacts;
}

void ContactFSV3Strategy::write(QString file, QString data)
{
    file = QString("");
    data = QString("");
}