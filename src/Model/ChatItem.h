#ifndef CHATITEM_H
#define CHATITEM_H

#include <QString>

class ChatItem
{
    private:
        long _timestamp;
        QString _address;
        QString _contact;
        QString _memo;
        QString _requestZaddr;
        QString _type; 
        QString _cid;
        QString _txid;
        bool _outgoing = false;

    public:
        ChatItem();
        ChatItem(long timestamp, QString address, QString contact, QString memo,QString requestZaddr, QString type, QString cid, QString txid);
        ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type,  QString cid, QString txid, bool outgoing);
        long getTimestamp();
        QString getAddress();
        QString getContact();
        QString getMemo();
        QString getRequestZaddr();
        QString getType();
        QString getCid();
        QString getTxid();
        bool isOutgoing();
        void setTimestamp(long timestamp);
        void setAddress(QString address);
        void setContact(QString contact);
        void setMemo(QString memo);
        void setRequestZaddr(QString requestZaddr);
        void setType(QString type);
        void setCid(QString cid);
        void setTxid(QString txid);
        void toggleOutgo();
        QString toChatLine();
        ~ChatItem();
};

#endif