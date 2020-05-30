// Copyright 2019-2020 The Hush developers
// GPLv3

#include "contactmodel.h"
#include "addressbook.h"
#include "mainwindow.h"
#include "chatmodel.h"
#include "requestdialog.h"
#include "ui_requestdialog.h"
#include "ui_hushrequest.h"
#include "settings.h"
#include "controller.h"



void ContactModel::renderContactList(QListView* view)
{    
     QStandardItemModel* contact = new QStandardItemModel();
         
	for(auto &c : AddressBook::getInstance()->getAllAddressLabels())
    {


          QString avatar = c.getAvatar();

             QStandardItem* Items1 = new QStandardItem(c.getName());
             Items1->setData(QIcon(avatar),Qt::DecorationRole);
             
             contact->appendRow(Items1); 
             view->setModel(contact);
             view->setIconSize(QSize(60,70));
             view->setUniformItemSizes(true);
             view->setDragDropMode(QAbstractItemView::DropOnly);      
             view->show();
             
           
        
    }
}

void MainWindow::showRequesthush() {
 
    Ui_hushrequest req;
    QDialog d(this);
    req.setupUi(&d);
        Settings::saveRestore(&d);

   QString label = ui->contactNameMemo->text();  
    for(auto &p : AddressBook::getInstance()->getAllAddressLabels())
    {

        if (p.getName() == label)

        {

            QString addr = p.getPartnerAddress();
            QString myzaddr = p.getMyAddress();

            req.txtFrom->setText(addr);
            req.lblAddressInfo->setText(myzaddr);
            // Amount textbox
    req.txtAmount->setValidator(this->getAmountValidator());
    QObject::connect(req.txtAmount, &QLineEdit::textChanged, [=] (auto text) {
        CAmount amount = CAmount::fromDecimalString(text);
     if (Settings::getInstance()->get_currency_name() == "USD") {
         req.txtAmountUSD->setText(amount.toDecimalUSDString());
    } else if (Settings::getInstance()->get_currency_name() == "EUR") {
        req.txtAmountUSD->setText(amount.toDecimalEURString());
    } else if (Settings::getInstance()->get_currency_name() == "BTC") {
        req.txtAmountUSD->setText(amount.toDecimalBTCString());
    } else if (Settings::getInstance()->get_currency_name() == "CNY") {
        req.txtAmountUSD->setText(amount.toDecimalCNYString());
    } else if (Settings::getInstance()->get_currency_name() == "RUB") {
        req.txtAmountUSD->setText(amount.toDecimalRUBString());
    } else if (Settings::getInstance()->get_currency_name() == "CAD") {
        req.txtAmountUSD->setText(amount.toDecimalCADString());
    } else if (Settings::getInstance()->get_currency_name() == "SGD") {
        req.txtAmountUSD->setText(amount.toDecimalSGDString());
    } else if (Settings::getInstance()->get_currency_name() == "CHF") {
        req.txtAmountUSD->setText(amount.toDecimalCHFString());
    } else if (Settings::getInstance()->get_currency_name() == "INR") {
        req.txtAmountUSD->setText(amount.toDecimalINRString());
    } else if (Settings::getInstance()->get_currency_name() == "GBP") {
        req.txtAmountUSD->setText(amount.toDecimalGBPString());
    } else if (Settings::getInstance()->get_currency_name() == "AUD") {
        req.txtAmountUSD->setText(amount.toDecimalBTCString());
        }
    });
    CAmount amount = CAmount::fromDecimalString(req.txtAmount->text());
    if (Settings::getInstance()->get_currency_name() == "USD") {
         req.txtAmountUSD->setText(amount.toDecimalUSDString());
    } else if (Settings::getInstance()->get_currency_name() == "EUR") {
        req.txtAmountUSD->setText(amount.toDecimalEURString());
    } else if (Settings::getInstance()->get_currency_name() == "BTC") {
        req.txtAmountUSD->setText(amount.toDecimalBTCString());
    } else if (Settings::getInstance()->get_currency_name() == "CNY") {
        req.txtAmountUSD->setText(amount.toDecimalCNYString());
    } else if (Settings::getInstance()->get_currency_name() == "RUB") {
        req.txtAmountUSD->setText(amount.toDecimalRUBString());
    } else if (Settings::getInstance()->get_currency_name() == "CAD") {
        req.txtAmountUSD->setText(amount.toDecimalCADString());
    } else if (Settings::getInstance()->get_currency_name() == "SGD") {
        req.txtAmountUSD->setText(amount.toDecimalSGDString());
    } else if (Settings::getInstance()->get_currency_name() == "CHF") {
        req.txtAmountUSD->setText(amount.toDecimalCHFString());
    } else if (Settings::getInstance()->get_currency_name() == "INR") {
        req.txtAmountUSD->setText(amount.toDecimalINRString());
    } else if (Settings::getInstance()->get_currency_name() == "GBP") {
        req.txtAmountUSD->setText(amount.toDecimalGBPString());
    } else if (Settings::getInstance()->get_currency_name() == "AUD") {
        req.txtAmountUSD->setText(amount.toDecimalBTCString());
        }
    req.txtMemo->setAcceptButton(req.buttonBox->button(QDialogButtonBox::Ok));
    req.txtMemo->setLenDisplayLabel(req.lblMemoLen);
    req.txtMemo->setMaxLen(400);

    req.txtFrom->setFocus();


        }
    }
            if (d.exec() == QDialog::Accepted) {
        // Construct a hush Payment URI with the data and pay it immediately.
        CAmount amount = CAmount::fromDecimalString(req.txtAmount->text());
        QString memoURI = "hush:" +  req.lblAddressInfo->text()
                    + "?amt=" + amount.toDecimalString()
                    + "&memo=" + QUrl::toPercentEncoding(req.txtMemo->toPlainText());

        QString sendURI = "hush:" + AddressBook::addressFromAddressLabel(req.txtFrom->text()) 
                    + "?amt=0.0001"
                    + "&memo=" + QUrl::toPercentEncoding(memoURI);

        // If the disclosed address in the memo doesn't have a balance, it will automatically fallback to the default
        // sapling address
        this->payhushURI(sendURI,  req.lblAddressInfo->text());



    }

}


