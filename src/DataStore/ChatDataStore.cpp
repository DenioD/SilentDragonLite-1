#include "ChatDataStore.h"

ChatDataStore* ChatDataStore::getInstance()
{
    if(!ChatDataStore::instanced)
    {
        ChatDataStore::instanced = true;
        ChatDataStore::instance = new ChatDataStore();
    }
                
    return ChatDataStore::instance;
}

void ChatDataStore::clear()
{
    this->data.clear();
}


void ChatDataStore::setData(QString key, ChatItem value)
{
    this->data[key] = value;
}

ChatItem ChatDataStore::getData(QString key)
{
    return this->data[key];
}

QString ChatDataStore::dump()
{
	return "";
}

std::map<QString, ChatItem> ChatDataStore::getAllRawChatItems()
{
    return this->data;
}

std::map<QString, ChatItem> ChatDataStore::getAllContactRequests()
{
    std::map<QString, ChatItem> filteredItems;
    for(auto &c: this->data)
    {
        if (
            (c.second.getType() == "cont") && 
            (c.second.isOutgoing() == false) && 
            (c.second.getMemo().startsWith("{"))
        ) 
        {
            filteredItems[c.first] = c.second;
        }
    }
    return filteredItems;
}


std::map<QString, ChatItem> ChatDataStore::getAllMemos()
{
    std::map<QString, ChatItem> filteredItems;
    for(auto &c: this->data)
    {
        if (

            (c.second.getMemo().startsWith("{") == false) &&
            (c.second.getMemo().isEmpty() == false) 
               
        ) 
        {
            filteredItems[c.first] = c.second;
        }
    }
    return filteredItems;
}


ChatDataStore* ChatDataStore::instance = nullptr;
bool ChatDataStore::instanced = false;