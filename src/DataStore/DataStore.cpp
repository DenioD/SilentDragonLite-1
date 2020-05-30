#include "DataStore.h"

SietchDataStore* DataStore::getSietchDataStore()
{
    return SietchDataStore::getInstance();
}

ChatDataStore* DataStore::getChatDataStore()
{
    return ChatDataStore::getInstance();
}

ContactDataStore* DataStore::getContactDataStore()
{
    return ContactDataStore::getInstance();
}