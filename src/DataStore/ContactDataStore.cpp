// Copyright 2019-2020 The Hush developers
// GPLv3

#include "ContactDataStore.h"
#include <string>

ContactDataStore* ContactDataStore::getInstance()
{
    if(!ContactDataStore::instanced)
    {
        ContactDataStore::instanced = true;
        ContactDataStore::instance = new ContactDataStore();
    }
                
    return ContactDataStore::instance;
}

void ContactDataStore::clear()
{
    this->data.clear();
}


void ContactDataStore::setData(QString key, ContactItem value)
{
    this->data[key] = value;
}

ContactItem ContactDataStore::getData(QString key)
{
    return this->data[key];
}

QString ContactDataStore::dump()
{
    QJsonObject contacts;
    contacts["count"] = (qint64)this->data.size();
    QJsonArray j = {};
    for (auto &c: this->data)
    {
        qDebug() << c.second.toQTString();
        c.second.toJson();
        j.push_back(c.second.toJson());
    }
    contacts["contacts"] = j;
    QJsonDocument jd_contacts = QJsonDocument(contacts);
    return QLatin1String(jd_contacts.toJson(QJsonDocument::Compact));

    // return QString::fromStdString(contacts.dump(4));
}

ContactDataStore* ContactDataStore::instance = nullptr;
bool ContactDataStore::instanced = false;
