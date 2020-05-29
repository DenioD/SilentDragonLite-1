#include "ContactFSV1BStrategy.h"


QList<ContactItem> ContactFSV1BStrategy::read(QString file)
{
    qDebug() << "Reading v1b addresses";
    QFile _file(file);
    _file.open(QIODevice::ReadOnly);
    QDataStream in(&_file);
    QList<ContactItem> contacts;

    QString version;
    in >> version;

    QList<QList<QString>> stuff;
    in >> stuff;
    qDebug() << "Stuff: " << stuff;
    ContactItem contact;
    for (int i=0; i < stuff.size(); i++) 
    {

        if(stuff[i].size() >= 4)
            contact = ContactItem(stuff[i][0], stuff[i][1], stuff[i][2], stuff[i][3]); // if sdl 1.2.x chat dev was used which we convert to v2

        else
            contact = ContactItem(stuff[i][0], stuff[i][1]);

        contacts.push_back(contact);
        qDebug() << "Contact: " << contact.toQTString();
    }
    
    _file.close();
    return contacts;
}

void ContactFSV1BStrategy::write(QString file, QString data)
{
    file = QString("");
    data = QString("");
}