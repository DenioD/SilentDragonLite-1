#include "SietchDataStore.h"

SietchDataStore* SietchDataStore::getInstance()
{
    if(!SietchDataStore::instanced)
    {
        SietchDataStore::instanced = true;
        SietchDataStore::instance = new SietchDataStore();
    }
                
    return SietchDataStore::instance;
}

void SietchDataStore::clear()
{
    this->data.clear();
}

void SietchDataStore::setData(QString key, QString value)
{
    this->data[key] = value;
}

QString SietchDataStore::getData(QString key)
{
    return this->data[key];
}

QString SietchDataStore::dump()
{
    return "";
}

SietchDataStore* SietchDataStore::instance = nullptr;
bool SietchDataStore::instanced = false;