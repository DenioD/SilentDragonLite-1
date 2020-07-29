// Copyright 2019-2020 The Hush developers
// GPLv3

#ifndef CHATITEM_H
#define CHATITEM_H

#include <QString>
using json = nlohmann::json;

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
        int _confirmations;
        bool _outgoing = false;
        bool _notarize = false;
        bool _iscontact = false;

    public:
        ChatItem();
        ChatItem(long timestamp, QString address, QString contact, QString memo,QString requestZaddr, QString type, QString cid, QString txid, int confirmations,  bool notarize, bool iscontact);
        ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type,  QString cid, QString txid, int confirmations, bool outgoing,  bool notarize, bool iscontact);
        long getTimestamp();
        QString getAddress();
        QString getContact();
        QString getMemo();
        QString getRequestZaddr();
        QString getType();
        QString getCid();
        QString getTxid();
        int getConfirmations();
        bool isOutgoing();
        bool isdouble();
        bool isNotarized();
        bool isContact();
        void setTimestamp(long timestamp);
        void setAddress(QString address);
        void setContact(QString contact);
        void setMemo(QString memo);
        void setRequestZaddr(QString requestZaddr);
        void setType(QString type);
        void setCid(QString cid);
        void setTxid(QString txid);
        void setConfirmations(int confirmations);
        void toggleOutgo();
        void notarized();
        void contact(bool iscontact);
        QString toChatLine();
        json toJson();
        ~ChatItem();
};

#endif
