// Copyright 2019-2020 The Hush developers
// GPLv3

#include "ContactRequest.h"

ContactRequest::ContactRequest() {}

ContactRequest::ContactRequest(QString sender, QString receiver, QString memo, QString cid, QString label, QString avatar)
{
    _senderAddress = sender;
    _receiverAddress = receiver;
    _memo = memo;
    _cid = cid;
    _label = label;
    _avatar = avatar;
}

QString ContactRequest::getSenderAddress()
{
    return _senderAddress;
}

QString ContactRequest::getReceiverAddress()
{
    return _receiverAddress;
}

QString ContactRequest::getMemo()
{
    return _memo;
}

QString ContactRequest::getCid()
{
    return _cid;
}

QString ContactRequest::getLabel()
{
    return _label;
}

QString ContactRequest::getAvatar()
{
    return _avatar;
}

void ContactRequest::setSenderAddress(QString address)
{
    _senderAddress = address;
}

void ContactRequest::setReceiverAddress(QString address)
{
    _receiverAddress = address;
}

void ContactRequest::setMemo(QString memo)
{
    _memo = memo;
}

void ContactRequest::setCid(QString cid)
{
    _cid = cid;
}

void ContactRequest::setLabel(QString label)
{
    _label = label;
}

void ContactRequest::setAvatar(QString avatar)
{
    _avatar = avatar;
}

QString ContactRequest::toString()
{
    return "sender: " + _senderAddress + " receiver: " + _receiverAddress + " memo: " + _memo + " cid: " + _cid + " label: " + _label + " avatar: " + _avatar; 
}

void ContactRequest::clear()
{
    _senderAddress = "";
    _receiverAddress = "";
    _memo = "";
    _cid = "";
    _label = "";
    _avatar = "";
}

ContactRequest::~ContactRequest()
{
   clear();
}
