// Copyright 2019-2020 The Hush developers
// GPLv3
#include "chatmodel.h"
#include "settings.h"
#include "ui_confirm.h"
#include "controller.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_requestContactDialog.h"
#include "addressbook.h"
#include "ui_memodialog.h"
#include "ui_contactrequest.h"
#include "addressbook.h"
#include <QtWidgets>
#include <QUuid>
#include "DataStore/DataStore.h"

using namespace std; 

ChatModel::ChatModel(std::map<QString, ChatItem> chatItems)
{
    this->chatItems = chatItems;
}

ChatModel::ChatModel(std::vector<ChatItem> chatItems)
{
   this->setItems(chatItems);
}

/*QString ChatModel::generateChatItemID(ChatItem item)
{
    QString key = QString::number(item.getTimestamp()) +  QString("-");
    key += QString(QCryptographicHash::hash(
        QString(
            QString::number(item.getTimestamp()) +  
            item.getAddress() + 
            item.getContact() + 
            item.getMemo()
        ).toUtf8()
        ,QCryptographicHash::Md5).toHex());
    return key;
}*/

std::map<QString, ChatItem> ChatModel::getItems()
{
    return this->chatItems;
}

void ChatModel::setItems(std::map<QString, ChatItem> items)
{
    this->chatItems = chatItems;
}

void ChatModel::setItems(std::vector<ChatItem> items)
{
    for(ChatItem c : items)
    {
        this->chatItems[ChatIDGenerator::getInstance()->generateID(c)] = c; //this->generateChatItemID(c)] = c;
    }
}

void ChatModel::clear()
{
    this->chatItems.clear();
}

void ChatModel::addMessage(ChatItem item)
{
    QString key = ChatIDGenerator::getInstance()->generateID(item); //this->generateChatItemID(item);
  //  qDebug() << "inserting chatitem with id: " << key;
    this->chatItems[key] = item;
}

void ChatModel::addMessage(QString timestamp, ChatItem item)
{
    QString key = ChatIDGenerator::getInstance()->generateID(item);//this->generateChatItemID(item);
    timestamp = "0";
    this->chatItems[key] = item;
}

void ChatModel::showMessages()
{
    for(auto &c : this->chatItems)
    {
        qDebug() << c.second.toChatLine();
    }
          
}
      
void MainWindow::renderContactRequest(){

        Ui_requestDialog requestContact;
        QDialog dialog(this);
        requestContact.setupUi(&dialog);
        Settings::saveRestore(&dialog);

       QStandardItemModel* contactRequest = new QStandardItemModel();

    {
            for (auto &c : DataStore::getChatDataStore()->getAllRawChatItems())
            {

                QStandardItem* Items = new QStandardItem(c.second.getAddress());
                contactRequest->appendRow(Items);
                requestContact.requestContact->setModel(contactRequest);
                requestContact.requestContact->show();
            }
        }

        QObject::connect(requestContact.requestContact, &QTableView::clicked, [&] () {

    for (auto &c : DataStore::getChatDataStore()->getAllRawChatItems()){//this->chatItems){
        QModelIndex index = requestContact.requestContact->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();
        QStandardItemModel* contactMemo = new QStandardItemModel();
           
        if  (c.second.isOutgoing() == false) {
            if (label_contact == c.second.getAddress()) {
                if(c.second.getMemo().startsWith("{")){

                }else{
          QStandardItem* Items = new QStandardItem(c.second.getMemo());
         contactMemo->appendRow(Items);
            requestContact.requestMemo->setModel(contactMemo);   
            requestContact.requestMemo->show();
           

            requestContact.requestZaddr->setText(c.second.getRequestZaddr());
            requestContact.requestCID->setText(c.second.getCid());
            requestContact.requestMyAddr->setText(c.second.getAddress());
            }
        }
    }
    
            }
   });
   
  QObject::connect(requestContact.pushButton, &QPushButton::clicked, [&] () {

            QString cid = requestContact.requestCID->text();
            auto addr = requestContact.requestZaddr->text().trimmed();
            QString newLabel = requestContact.requestLabel->text().trimmed();
            auto myAddr = requestContact.requestMyAddr->text().trimmed();

            QString avatar = QString(":/icons/res/") + requestContact.comboBoxAvatar->currentText() + QString(".png");

             if (addr.isEmpty() || newLabel.isEmpty()) 
        {
             QMessageBox::critical(
                this, 
                QObject::tr("Address or Label Error"), 
                QObject::tr("Address or Label cannot be empty"), 
                QMessageBox::Ok
                );
            return;
        }

        // Test if address is valid.
        if (!Settings::isValidAddress(addr)) 
        {
          QMessageBox::critical(
                this, 
                QObject::tr("Address Format Error"), 
                QObject::tr("%1 doesn't seem to be a valid hush address.").arg(addr), 
                QMessageBox::Ok
            );
            return;
        } 

                qDebug()<<"Beginn kopiert" <<cid << addr << newLabel << myAddr;
                AddressBook::getInstance()->addAddressLabel(newLabel, addr, myAddr, cid, avatar);

                  QMessageBox::information(this, "Added Contact","successfully added your new contact. You can now Chat with this contact");      
            
    });
       

 dialog.exec();
}

void ChatModel::addCid(QString tx, QString cid)
{
    this->cidMap[tx] = cid;
}

void ChatModel::addrequestZaddr(QString tx, QString requestZaddr)
{
    this->requestZaddrMap[tx] = requestZaddr;
}

QString ChatModel::getCidByTx(QString tx)
{
    for(auto& pair : this->cidMap)
    {

    }

    if(this->cidMap.count(tx) > 0)
    {
        return this->cidMap[tx];
    }

    return QString("0xdeadbeef");
}

QString ChatModel::getrequestZaddrByTx(QString tx)
{
    for(auto& pair : this->requestZaddrMap)
    {

    }

    if(this->requestZaddrMap.count(tx) > 0)
    {
        return this->requestZaddrMap[tx];
    }

    return QString("0xdeadbeef");
}

void ChatModel::killCidCache()
{
    this->cidMap.clear();
}

void ChatModel::killrequestZaddrCache()
{
    this->requestZaddrMap.clear();
}

QString MainWindow::createHeaderMemo(QString type, QString cid, QString zaddr,  int version=0, int headerNumber=1)
{
    
    QString header="";
    QJsonDocument j;
    QJsonObject h;
    // We use short keynames to use less space for metadata and so allow
    // the user to send more actual data in memos
    h["h"]   = headerNumber;    // header number
    h["v"]   = version;         // HushChat version
    h["z"]   = zaddr;           // zaddr to respond to
    h["cid"] = cid;             // conversation id
    h["t"] = type;       // Memo or incoming contact request

    j.setObject(h);
    header = j.toJson();
    qDebug() << "made header=" << header;
    return header;
   
}

// Create a Tx from the current state of the Chat page. 
Tx MainWindow::createTxFromChatPage() {
   Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
  {
       
        QString amtStr = "0";
        CAmount amt;  

         
            amt = CAmount::fromDecimalString("0");
            totalAmt = totalAmt + amt;

  
    for(auto &c : AddressBook::getInstance()->getAllAddressLabels())

     if (ui->contactNameMemo->text().trimmed() == c.getName()) {
     
            QString cid = c.getCid();
            QString myAddr = c.getMyAddress();
            QString type = "Memo";
            QString addr = c.getPartnerAddress();
           
     
        QString hmemo= createHeaderMemo(type,cid,myAddr);
        QString memo = ui->memoTxtChat->toPlainText().trimmed();
        

       // ui->memoSizeChat->setLenDisplayLabel();// Todo -> activate lendisplay for chat
     
     tx.toAddrs.push_back(ToFields{addr, amt, hmemo});
     tx.toAddrs.push_back(ToFields{addr, amt, memo});

     

         qDebug() << "pushback chattx";
   } }

    tx.fee = Settings::getMinerFee();

     return tx;

     qDebug() << "ChatTx created";
}

void MainWindow::sendChatButton() {

////////////////////////////Todo: Check if a Contact is selected//////////

    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.

    // Memos can only be used with zAddrs. So check that first
   // for(auto &c : AddressBook::getInstance()->getAllAddressLabels())

      if (ui->contactNameMemo->text().trimmed().isEmpty() || ui->memoTxtChat->toPlainText().trimmed().isEmpty()) {
     
  // auto addr = "";
  //  if (! Settings::isZAddress(AddressBook::addressFromAddressLabel(addr->text()))) {
        QMessageBox msg(QMessageBox::Critical, tr("You have to select a contact and insert a Memo"),
        tr("You have selected no Contact from Contactlist,\n")  + tr("\nor your Memo is empty"),
        QMessageBox::Ok, this);

        msg.exec();
        return;
    }

    Tx tx = createTxFromChatPage();

    QString error = doSendChatTxValidations(tx);

    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(QMessageBox::Critical, tr("Message Error"), error,
                        QMessageBox::Ok, this);

        msg.exec();

        // abort the Tx
        return;
        qDebug() << "Tx aborted";
    }

        // Create a new Dialog to show that we are computing/sending the Tx
        auto d = new QDialog(this);
        auto connD = new Ui_ConnectionDialog();
        connD->setupUi(d);
        QMovie *movie1 = new QMovie(":/img/res/silentdragonlite-animated.gif");;
        QMovie *movie2 = new QMovie(":/img/res/silentdragonlite-animated-dark.gif");;
        auto theme = Settings::getInstance()->get_theme_name();
        if (theme == "dark" || theme == "midnight") {
            movie2->setScaledSize(QSize(512,512));
            connD->topIcon->setMovie(movie2);
            movie2->start();
        } else {
            movie1->setScaledSize(QSize(512,512));
            connD->topIcon->setMovie(movie1);
            movie1->start();
        }

        connD->status->setText(tr("Please wait..."));
        connD->statusDetail->setText(tr("Your Message will be send"));

        d->show();
        ui->memoTxtChat->clear();

        // And send the Tx
        rpc->executeTransaction(tx, 
            [=] (QString txid) { 
                ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);

                connD->status->setText(tr("Done!"));
                connD->statusDetail->setText(txid);

                QTimer::singleShot(1000, [=]() {
                    d->accept();
                    d->close();
                    delete connD;
                    delete d;
                    
                  });
                
                // Force a UI update so we get the unconfirmed Tx
              //  rpc->refresh(true);
                ui->memoTxtChat->clear();

            },
            // Errored out
            [=] (QString opid, QString errStr) {
                ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
                
                d->accept();
                d->close();
                delete connD;
                delete d;

                if (!opid.isEmpty())
                    errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            

                QMessageBox::critical(this, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);            
            }
        );
    }        

QString MainWindow::doSendChatTxValidations(Tx tx) {
    // Check to see if we have enough verified funds to send the Tx.

    CAmount total;
    for (auto toAddr : tx.toAddrs) {
        if (!Settings::isValidAddress(toAddr.addr)) {
            QString addr = (toAddr.addr.length() > 100 ? toAddr.addr.left(100) + "..." : toAddr.addr);
            return QString(tr("Recipient Address ")) % addr % tr(" is Invalid");
        }

        // This technically shouldn't be possible, but issue #62 seems to have discovered a bug
        // somewhere, so just add a check to make sure. 
        if (toAddr.amount.toqint64() < 0) {
            return QString(tr("Amount for address '%1' is invalid!").arg(toAddr.addr));
        }

        total = total + toAddr.amount;
    }
    total = total + tx.fee;

    auto available = rpc->getModel()->getAvailableBalance();

    if (available < total) {
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 3 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}

void::MainWindow::addContact() {

    Ui_Dialog request;
    QDialog dialog(this);
    request.setupUi(&dialog);
    Settings::saveRestore(&dialog);
    
        bool sapling = true;
        rpc->createNewZaddr(sapling, [=] (json reply) {
            QString myAddr = QString::fromStdString(reply.get<json::array_t>()[0]);
            QString cid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            ui->listReceiveAddresses->insertItem(0, myAddr); 
            ui->listReceiveAddresses->setCurrentIndex(0);

            qDebug() << "new generated myAddr" << myAddr;
            request.myzaddr->setText(myAddr);
            request.cid->setText(cid);

            });
            

     QObject::connect(request.sendRequestButton, &QPushButton::clicked, [&] () {

            QString cid = request.cid->text();
            auto addr = request.zaddr->text().trimmed();
            QString newLabel = request.labelRequest->text().trimmed();
            auto myAddr = request.myzaddr->text().trimmed();

            QString avatar = QString(":/icons/res/") + request.comboBoxAvatar->currentText() + QString(".png");

             if (addr.isEmpty() || newLabel.isEmpty()) 
        {
             QMessageBox::critical(
                this, 
                QObject::tr("Address or Label Error"), 
                QObject::tr("Address or Label cannot be empty"), 
                QMessageBox::Ok
                );
            return;
        }

        // Test if address is valid.
        if (!Settings::isValidAddress(addr)) 
        {
          QMessageBox::critical(
                this, 
                QObject::tr("Address Format Error"), 
                QObject::tr("%1 doesn't seem to be a valid hush address.").arg(addr), 
                QMessageBox::Ok
            );
            return;
        } 

        ///////Todo: Test if label allready exist!

        ////// Success, so show it
            AddressBook::getInstance()->addAddressLabel(newLabel, addr, myAddr, cid, avatar);
              QMessageBox::information(
                this, 
                QObject::tr("Added Contact"), 
                QObject::tr("successfully added your new contact").arg(newLabel), 
                QMessageBox::Ok
            );
            return;

    });

       
   dialog.exec();
}

// Create a Tx for a contact Request 
Tx MainWindow::createTxForSafeContactRequest() {

    Tx tx; 
        
    // For each addr/amt in the Chat tab
  {
       CAmount totalAmt;
        QString amtStr = "0";
        CAmount amt;  

         
            amt = CAmount::fromDecimalString("0");
            totalAmt = totalAmt + amt;

  
    for(auto &c : AddressBook::getInstance()->getAllAddressLabels())
    if (ui->contactNameMemo->text().trimmed() == c.getName()) {

          
            QString cid = c.getCid();
            QString myAddr = c.getMyAddress();
            QString type = "cont";
            QString addr = c.getPartnerAddress();     
     
            QString hmemo= createHeaderMemo(type,cid,myAddr);
            QString memo = ui->memoTxtChat->toPlainText().trimmed();

            tx.toAddrs.push_back(ToFields{addr, amt, memo});
            tx.toAddrs.push_back(ToFields{addr, amt, hmemo});
            

                qDebug() << "pushback chattx";
            
     }
    
      tx.fee = Settings::getMinerFee();

     return tx;

     qDebug() << "RequestTx created";


  }
  
}

void MainWindow::ContactRequest() {

    if (ui->contactNameMemo->text().trimmed().isEmpty() || ui->memoTxtChat->toPlainText().trimmed().isEmpty()) {
     
  // auto addr = "";
  //  if (! Settings::isZAddress(AddressBook::addressFromAddressLabel(addr->text()))) {
        QMessageBox msg(QMessageBox::Critical, tr("You have to select a contact and insert a Memo"),
        tr("You have selected no Contact from Contactlist,\n")  + tr("\nor your Memo is empty"),
        QMessageBox::Ok, this);

        msg.exec();
        return;
    }

  Tx tx = createTxForSafeContactRequest();

    QString error = doSendRequestTxValidations(tx);

    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(QMessageBox::Critical, tr("Message Error"), error,
                        QMessageBox::Ok, this);

        msg.exec();

        // abort the Tx
        return;
        qDebug() << "Tx aborted";
    }

        // Create a new Dialog to show that we are computing/sending the Tx
        auto d = new QDialog(this);
        auto connD = new Ui_ConnectionDialog();
        connD->setupUi(d);
        QMovie *movie1 = new QMovie(":/img/res/silentdragonlite-animated.gif");;
        QMovie *movie2 = new QMovie(":/img/res/silentdragonlite-animated-dark.gif");;
        auto theme = Settings::getInstance()->get_theme_name();
        if (theme == "dark" || theme == "midnight") {
            movie2->setScaledSize(QSize(512,512));
            connD->topIcon->setMovie(movie2);
            movie2->start();
        } else {
            movie1->setScaledSize(QSize(512,512));
            connD->topIcon->setMovie(movie1);
            movie1->start();
        }

        connD->status->setText(tr("Please wait..."));
        connD->statusDetail->setText(tr("Your Contact will be send"));

        d->show();
        ui->memoTxtChat->clear();

        // And send the Tx
        rpc->executeTransaction(tx, 
            [=] (QString txid) { 
                ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);

                connD->status->setText(tr("Done!"));
                connD->statusDetail->setText(txid);

                QTimer::singleShot(1000, [=]() {
                    d->accept();
                    d->close();
                    delete connD;
                    delete d;
                    
                  });
                
                // Force a UI update so we get the unconfirmed Tx
              //  rpc->refresh(true);
                ui->memoTxtChat->clear();
                rpc->refresh(true);

            },
            // Errored out
            [=] (QString opid, QString errStr) {
                ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
                
                d->accept();
                d->close();
                delete connD;
                delete d;

                if (!opid.isEmpty())
                    errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            

                QMessageBox::critical(this, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);            
            }
        );
       
    }        


QString MainWindow::doSendRequestTxValidations(Tx tx) {
    // Check to see if we have enough verified funds to send the Tx.

    CAmount total;
    for (auto toAddr : tx.toAddrs) {
        if (!Settings::isValidAddress(toAddr.addr)) {
            QString addr = (toAddr.addr.length() > 100 ? toAddr.addr.left(100) + "..." : toAddr.addr);
            return QString(tr("Recipient Address ")) % addr % tr(" is Invalid");
        }

        // This technically shouldn't be possible, but issue #62 seems to have discovered a bug
        // somewhere, so just add a check to make sure. 
        if (toAddr.amount.toqint64() < 0) {
            return QString(tr("Amount for address '%1' is invalid!").arg(toAddr.addr));
        }

        total = total + toAddr.amount;
    }
    total = total + tx.fee;

    auto available = rpc->getModel()->getAvailableBalance();

    if (available < total) {
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 5 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}