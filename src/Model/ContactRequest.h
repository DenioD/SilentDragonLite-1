// Copyright 2019-2020 The Hush developers
// GPLv3

#ifndef CONTACTREQUEST_H
#define CONTACTREQUEST_H

#include <QString>
using json = nlohmann::json;

class ContactRequest
{
    private:
        QString _senderAddress;
        QString _receiverAddress;
        QString _memo;
        QString _cid;
        QString _label;
        QString _avatar;

    public:
        ContactRequest();
        ContactRequest(QString sender, QString receiver, QString memo, QString cid, QString label, QString avatar);
        QString getSenderAddress();
        QString getReceiverAddress();
        QString getMemo();
        QString getCid();
        QString getLabel();
        QString getAvatar();
        void setSenderAddress(QString address);
        void setReceiverAddress(QString contact);
        void setMemo(QString memo);
        void setCid(QString cid);
        void setLabel(QString label);
        void setAvatar(QString avatar);
        QString toString();
        void clear();
        ~ContactRequest();
};

#endif