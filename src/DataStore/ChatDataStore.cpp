// Copyright 2019-2020 The Hush developers
// GPLv3

#include "ChatDataStore.h"
#include "addressbook.h"
#include "chatmodel.h"


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
	json chats;
    chats["count"] = this->data.size();
    json j = {};
    for (auto &c: this->data)
    {
        j.push_back(c.second.toJson());
    }
    chats["chatitems"] = j;
	return QString::fromStdString(chats.dump());
}

std::map<QString, ChatItem> ChatDataStore::getAllRawChatItems()
{
    return this->data;
}

std::map<QString, ChatItem> ChatDataStore::getAllNewContactRequests()
{
    std::map<QString, ChatItem> filteredItems;
  
    for(auto &c: this->data)   
    {
        if (
            (c.second.isOutgoing() == false) &&
            (c.second.getType() == "Cont")  &&
            (c.second.isContact() == false)           
        ) 
        
        {
            filteredItems[c.first] = c.second;
        }
    }
    return filteredItems;
}

std::map<QString, ChatItem> ChatDataStore::getAllOldContactRequests()
{
    std::map<QString, ChatItem> filteredItems;
    for(auto &p : AddressBook::getInstance()->getAllAddressLabels())
    for(auto &c: this->data)
    {
        if (
            (c.second.isOutgoing() == false) &&
            (c.second.getType() == "Cont") &&  
            (p.getPartnerAddress() == c.second.getRequestZaddr())
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