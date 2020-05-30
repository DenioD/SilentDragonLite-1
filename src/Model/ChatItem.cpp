// Copyright 2019-2020 The Hush developers
// GPLv3

#include "ChatItem.h"

ChatItem::ChatItem() {}

ChatItem::ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type, QString cid, QString txid, int confirmations, bool notarize,  bool iscontact)
{
    _timestamp = timestamp;
    _address = address;
    _contact = contact;
    _memo = memo;
    _requestZaddr = requestZaddr;
    _type = type;
    _cid = cid;
    _txid = txid;
    _confirmations = confirmations;
    _outgoing = false;
    _notarize = notarize;
    _iscontact = iscontact;
}

ChatItem::ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type, QString cid, QString txid, int confirmations, bool outgoing, bool notarize,  bool iscontact)
{
    _timestamp = timestamp;
    _address = address;
    _contact = contact;
    _memo = memo;
    _requestZaddr = requestZaddr;
    _type = type;
    _cid = cid;
    _txid = txid;
    _confirmations = confirmations;
    _outgoing = outgoing;
    _notarize = notarize;
    _iscontact = iscontact;
     
}

long ChatItem::getTimestamp()
{
    return _timestamp;
}

QString ChatItem::getAddress()
{
    return _address;
}

QString ChatItem::getContact()
{
    return _contact;
}

QString ChatItem::getMemo()
{
    return _memo;
}

QString ChatItem::getRequestZaddr()
{
    return _requestZaddr;
}
QString ChatItem::getType()
{
    return _type;
}

QString ChatItem::getCid()
{
    return _cid;
}

QString ChatItem::getTxid()
{
    return _txid;
}
int ChatItem::getConfirmations()
{
    return _confirmations;
}

bool ChatItem::isOutgoing()
{
    return _outgoing;
}

bool ChatItem::isNotarized()
{
    return _notarize;
}

bool ChatItem::isContact()
{
    return _iscontact;
}

void ChatItem::setTimestamp(long timestamp)
{
    _timestamp = timestamp;
}

void ChatItem::setAddress(QString address)
{
    _address = address;
}

void ChatItem::setContact(QString contact)
{
    _contact = contact;
}

void ChatItem::setMemo(QString memo)
{
    _memo = memo;
}

void ChatItem::setRequestZaddr(QString requestZaddr)
{
    _requestZaddr = requestZaddr;
}

void ChatItem::setType(QString type)
{
    _type = type;
}

void ChatItem::setCid(QString cid)
{
    _cid = cid;
}
void ChatItem::setTxid(QString txid)
{
    _txid = txid;
}
void ChatItem::setConfirmations(int confirmations)
{
    _confirmations = confirmations;
}

void ChatItem::toggleOutgo()
{
    _outgoing = true;
}
void ChatItem::notarized()
{
    _notarize = false;
}

void ChatItem::contact(bool iscontact)
{
    _iscontact = iscontact;
}


QString ChatItem::toChatLine()
{
    QDateTime myDateTime;
    QString lock;
    myDateTime.setTime_t(_timestamp);

    if (_notarize == true)

    {

        lock = "<b> <img src=':/icons/res/lock_orange.png'><b>";

    }else{
        
         lock = "<b> <img src=':/icons/res/unlocked.png'><b>";
        }
        if ((_confirmations > 0) && (_notarize == false))
        
        {

        lock = "<b> <img src=':/icons/res/lock_green.png'><b>";
        }
    

qDebug()<<_notarize;
    QString line = QString("<small>") + myDateTime.toString("dd.MM.yyyy hh:mm");
    line += QString(lock) + QString("</small>");
    line += QString("<p>") + _memo.toHtmlEscaped() + QString("</p>");
    return line;
}

json ChatItem::toJson()
{
    json j;
    j["_timestamp"] = _timestamp;
    j["_address"] = _address.toStdString();
    j["_contact"] = _contact.toStdString();
    j["_memo"] = _memo.toStdString();
    j["_requestZaddr"] = _requestZaddr.toStdString();
    j["_type"] = _type.toStdString();
    j["_cid"] = _cid.toStdString();
    j["_txid"] = _txid.toStdString();
    j["_confirmations"] = _confirmations;
    j["_outgoing"] = _outgoing;
    return j;
}

ChatItem::~ChatItem()
{

}