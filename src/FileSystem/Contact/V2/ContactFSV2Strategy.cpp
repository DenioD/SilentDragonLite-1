#include "ContactFSV2Strategy.h"


QList<ContactItem> ContactFSV2Strategy::read(QString file)
{
    qDebug() << "Reading v2 addresses";
    QFile _file(file);
    _file.open(QIODevice::ReadOnly);
    QDataStream in(&_file);
    QList<ContactItem> contacts;

    QString version;
    in >> version;

    QList<QList<QString>> stuff;
    in >> stuff;
    for (int i= 0; i < stuff.size(); i++) 
    {
        qDebug() << stuff[i].size();
        ContactItem contact;
        if(stuff[i].size() == 4)
        {
            contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3]);
        }
        else
        {
            contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3],stuff[i][4]);
        }
                    
        qDebug() << contact.toQTString();
        contacts.push_back(contact);
    }

    _file.close();
    return contacts;
}

void ContactFSV2Strategy::write(QString file, QString data)
{
    file = QString("");
    data = QString("");
}