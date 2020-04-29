#include "contactmodel.h"
#include "addressbook.h"
#include "mainwindow.h"

void ContactModel::renderContactList(QListWidget* view)
{
    while(view->count() > 0)
    {
        view->takeItem(0);
    }
	for(auto &c : AddressBook::getInstance()->getAllAddressLabels())
    {
    	view->addItem(c.getPartnerAddress());
        
        
    }
}