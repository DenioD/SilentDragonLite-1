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

QString ChatDataStore::getPassword()
{

    return _password;
}

QString ChatDataStore::getSendZaddr()
{

    return _zaddr;
}

void ChatDataStore::setSendZaddr(QString zaddr)
{

    _zaddr = zaddr;
}

void ChatDataStore::setPassword(QString password)
{

    _password = password;
}

QString ChatDataStore::dump()
{
    QJsonObject chats;
    chats["count"] = (qint64)this->data.size();
    QJsonArray j;
    for (auto &c: this->data)
    {
        j.push_back(c.second.toJson());
    }
    chats["chatitems"] = j;
    QJsonDocument jd_chats = QJsonDocument(chats);
    return QLatin1String(jd_chats.toJson(QJsonDocument::Compact));
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
            (c.second.isContact() == false) &&
            (c.second.getMemo().startsWith("{"))  
              
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
 
    for(auto &c: this->data)
    {
        if (
            (c.second.isOutgoing() == false) &&
            (c.second.getType() == "Cont") &&  
            (c.second.isContact() == true) &&
            (c.second.getMemo().startsWith("{")) 
        ) 
        {
            filteredItems[c.first] = c.second;
        }
    }
    return filteredItems;
}

std::map<QString, ChatItem> ChatDataStore::getAllCashMemosIncoming()
{
    std::map<QString, ChatItem> filteredItems;
  
    for(auto &c: this->data)   
    {
        if (
            (c.second.isOutgoing() == false) &&
            (c.second.getType() == "Money")  &&
            (c.second.getMemo().startsWith("{"))  
              
        ) 
        
        {
            filteredItems[c.first] = c.second;
        }
    }
    return filteredItems;
}

std::map<QString, ChatItem> ChatDataStore::getAllCashMemosOutgoing()
{
    std::map<QString, ChatItem> filteredItems;
  
    for(auto &c: this->data)   
    {
        if (
            (c.second.isOutgoing() == true) &&
            (c.second.getType() == "Money")  &&
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
