#include "ChatItem.h"

ChatItem::ChatItem() {}

ChatItem::ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type, QString cid, QString txid)
{
    _timestamp = timestamp;
    _address = address;
    _contact = contact;
    _memo = memo;
    _requestZaddr = requestZaddr;
    _type = type;
    _cid = cid;
    _txid = txid;
    _outgoing = false;
}

ChatItem::ChatItem(long timestamp, QString address, QString contact, QString memo, QString requestZaddr, QString type, QString cid, QString txid, bool outgoing)
{
    _timestamp = timestamp;
    _address = address;
    _contact = contact;
    _memo = memo;
    _requestZaddr = requestZaddr;
    _type = type;
    _cid = cid;
    _txid = txid;
    _outgoing = outgoing;
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

bool ChatItem::isOutgoing()
{
    return _outgoing;
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

void ChatItem::toggleOutgo()
{
    _outgoing = true;
}

QString ChatItem::toChatLine()
{
    QDateTime myDateTime;
    myDateTime.setTime_t(_timestamp);
    QString line = QString("[") + myDateTime.toString("d.M.yy hh:mm") + QString("] ");
    line += QString("") + QString(_memo) + QString("\n\n");
    return line;
}

ChatItem::~ChatItem()
{
    /*delete timestamp;
            delete address;
            delete contact;
            delete memo;
            delete outgoing;*/
}