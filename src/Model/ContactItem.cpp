// Copyright 2019-2020 The Hush developers
// GPLv3
#include "ContactItem.h"
#include "chatmodel.h"
#include "Model/ChatItem.h"
#include "controller.h"

ContactItem::ContactItem() {}

ContactItem::ContactItem(QString name, QString partnerAddress)
{
    _name = name;
    _partnerAddress = partnerAddress;
}

ContactItem::ContactItem(QString name, QString partnerAddress, QString myAddress, QString cid)
{
    _name = name;
    _myAddress = myAddress;
    _partnerAddress = partnerAddress;
    _cid = cid;

}

ContactItem::ContactItem(QString name, QString partnerAddress, QString myAddress, QString cid, QString avatar)
{
    _name = name;
    _myAddress = myAddress;
    _partnerAddress = partnerAddress;
    _cid = cid;
    _avatar = avatar; 
 
}

QString ContactItem::getName() const
{
    return _name;
}

QString ContactItem::getMyAddress() const
{
    return _myAddress;
}

QString ContactItem::getPartnerAddress() const
{
    return _partnerAddress;
}

QString ContactItem::getCid() const
{
    return _cid;
}

QString ContactItem::getAvatar() const
{
    return _avatar;
}

void ContactItem::setName(QString name)
{
    _name = name;
}

void ContactItem::setMyAddress(QString myAddress)
{
    _myAddress = myAddress;
}

void ContactItem::setPartnerAddress(QString partnerAddress)
{
    _partnerAddress = partnerAddress;
}

void ContactItem::setcid(QString cid)
{
    _cid = cid;
}
void ContactItem::setAvatar(QString avatar)
{
    _avatar = avatar;
}

QString ContactItem::toQTString()
{
    return _name + "|" + _partnerAddress + "|" + _myAddress + "|" + _cid + "|" + _avatar;
}

json ContactItem::toJson()
{
    json j;
    j["_myAddress"] = _myAddress.toStdString();
    j["_partnerAddress"] = _partnerAddress.toStdString();
    j["_name"] = _name.toStdString();
    j["_cid"] = _cid.toStdString();
    j["_avatar"] = _avatar.toStdString();
    return j;
}