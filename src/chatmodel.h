#ifndef CHATMODEL_H
#define CHATMODEL_H
#include <QString>
#include <map>
#include <vector>
#include <QListWidget>
#include "precompiled.h"

class ChatItem
{
    private:
        long _timestamp;
        QString _address;
        QString _contact;
        QString _memo; 
        bool _outgoing = false;

    public:
        ChatItem() {}

        ChatItem(long timestamp, QString address, QString contact, QString memo)
        {
            _timestamp = timestamp;
            _address = address;
            _contact = contact;
            _memo = memo;
            _outgoing = false;

        }

        ChatItem(long timestamp, QString address, QString contact, QString memo, bool outgoing)
        {
            _timestamp = timestamp;
            _address = address;
            _contact = contact;
            _memo = memo;
            _outgoing = outgoing;

        }

        long getTimestamp()
        {
            return _timestamp;
        }

        QString getAddress()
        {
            return _address;
        }

        QString getContact()
        {
            return _contact;
        }

        QString getMemo()
        {
            return _memo;
        }

        bool isOutgoing()
        {
            return _outgoing;
        }

        void setTimestamp(long timestamp)
        {
            _timestamp = timestamp;
        }

        void setAddress(QString address)
        {
            _address = address;
        }

        void setContact(QString contact)
        {
            _contact = contact;
        }

        void setMemo(QString memo)
        {
            _memo = memo;
        }

        void toggleOutgo()
        {
            _outgoing = true;
        }

        ~ChatItem()
        {
            /*delete timestamp;
            delete address;
            delete contact;
            delete memo;
            delete outgoing;*/
        }
};

class ChatModel
{
    private:
        std::map<long, ChatItem> chatItems; 
        QTableView* parent;

    public:
        ChatModel() {};
        ChatModel(std::map<long, ChatItem> chatItems);
        ChatModel(std::vector<ChatItem> chatItems);
        std::map<long, ChatItem> getItems();
        void setItems(std::map<long, ChatItem> items);
        void setItems(std::vector<ChatItem> items);
        void renderChatBox(QListWidget &view);
        void renderChatBox(QListWidget *view);
        void showMessages();
        void clear();
        void addMessage(ChatItem item);
        void addMessage(long timestamp, ChatItem item);
};

#endif