#ifndef CONTACTITEM_H
#define CONTACTITEM_H

#include <vector>
#include <QString>

class ContactItem
{
private:
    QString _myAddress;
    QString _partnerAddress;
    QString _name;
    QString _cid;
    QString _avatar;

public:
    ContactItem();
    ContactItem(QString name, QString partnerAddress, QString myAddress, QString cid, QString avatar);
    QString getName() const;
    QString getMyAddress() const;
    QString getPartnerAddress() const;
    QString getCid() const;
    QString getAvatar() const;
    void setName(QString name);
    void setMyAddress(QString myAddress);
    void setPartnerAddress(QString partnerAddress);
    void setcid(QString cid);
    void setAvatar(QString avatar);
    QString toQTString();
};

#endif