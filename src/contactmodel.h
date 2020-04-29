#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <vector>
#include <QString>
#include <QListWidget>

class ContactItem
{
	private:
		QString _myAddress;
		QString _partnerAddress;
		QString _name;
		QString _cid;

	public:
		ContactItem();
		ContactItem(QString name, QString partnerAddress, QString myAddress, QString cid)
		{
			_name = name;
			_myAddress = myAddress;
			_partnerAddress = partnerAddress;
			_cid = cid;
		}

		QString getName() const
		{
			return _name;
		}

		QString getMyAddress() const
		{
			return _myAddress;
		}

		QString getPartnerAddress() const
		{
			return _partnerAddress;
		}

		QString getCid() const
		{
			return _cid;
		}

		void setName(QString name)
		{
			_name = name;
		}

		void setMyAddress(QString myAddress)
		{
			_myAddress = myAddress;
		}

		void setPartnerAddress(QString partnerAddress)
		{
			_partnerAddress = partnerAddress;
		}

		void setcid(QString cid)
		{
			_cid = cid;
		}

		QString toQTString()
		{
			return _name +  "|" +  _partnerAddress + "|" + _myAddress + "|" + _cid;
		}

};

class ContactModel
{
	public:
		ContactModel() {}
		void renderContactList(QListWidget* view);
};

#endif