// Copyright 2019-2020 The Hush developers
// GPLv3
#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include "Model/ContactItem.h"
#include <QListWidget>
#include "mainwindow.h"


class ContactModel

{
	public:
			MainWindow*                 main;
	    std::map<QString, QString> AddressMap;
    
    QString getContactbyAddress(QString addr);
	void addLabel(QString addr, QString label);

		ContactModel() {}
		void renderContactList(QListView* view);
		
			
};

#endif