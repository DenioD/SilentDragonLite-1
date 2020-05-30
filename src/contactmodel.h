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

		ContactModel() {}
		void renderContactList(QListView* view);
			
};

#endif