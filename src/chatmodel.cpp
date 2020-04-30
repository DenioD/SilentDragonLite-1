#include "chatmodel.h"
#include "settings.h"
#include "ui_confirm.h"
#include "controller.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addressbook.h"
#include "ui_memodialog.h"
#include "addressbook.h"
#include <QUuid>


ChatModel::ChatModel(std::map<long, ChatItem> chatItems)
{
    this->chatItems = chatItems;
}

ChatModel::ChatModel(std::vector<ChatItem> chatItems)
{
   this->setItems(chatItems);
}

std::map<long, ChatItem> ChatModel::getItems()
{
    return this->chatItems;
}

void ChatModel::setItems(std::map<long, ChatItem> items)
{
    this->chatItems = chatItems;
}

void ChatModel::setItems(std::vector<ChatItem> items)
{
    for(ChatItem c : items)
    {
        this->chatItems[c.getTimestamp()] = c;
    }
}

void ChatModel::clear()
{
    this->chatItems.clear();
}

void ChatModel::addMessage(ChatItem item)
{
    this->chatItems[item.getTimestamp()] = item;
}

void ChatModel::addMessage(long timestamp, ChatItem item)
{
    this->chatItems[timestamp] = item;
}

void ChatModel::showMessages()
{
    for(auto &c : this->chatItems)
    {
        qDebug() << "[" << c.second.getTimestamp() << "] " << "<" << c.second.getAddress() << "> :" << c.second.getMemo(); 
    }
}



void ChatModel::renderChatBox(QListWidget &view)
{
    qDebug() << "not implemented yet";
}

void ChatModel::renderChatBox(QListWidget *view)
{
    QString line = "";
    while(view->count() > 0)
    {
        view->takeItem(0);
    }
  
    for(auto &c : this->chatItems)
    {

        if (c.second.getMemo().startsWith("{\n    \"c\": \"true\""))
        {
            // Render a incoming contact Request
                
        }
      
        if (c.second.getMemo().startsWith("{\n    \"c\": \"false\""))
        {
            // we dont want to render this
            
        }
            
        if (c.second.getMemo().startsWith("{") == false)
        { 
            //TOdo and is selected in Contact Widget - 
            QDateTime myDateTime;
            myDateTime.setTime_t(c.second.getTimestamp());
            //qDebug() << "[" << myDateTime.toString("dd.MM.yyyy hh:mm:ss ") << "] " << "<" << c.second.getAddress() << "> :" << c.second.getMemo(); 
            line += QString("[") + myDateTime.toString("dd.MM.yyyy hh:mm:ss ") +  QString("] ");
            line += QString("<") + QString(c.second.getContact()) + QString("> :\n");
            line += QString(c.second.getMemo()) + QString("\n");
            view->addItem(line);
            line ="";
        }

    }
}

QString MainWindow::createHeaderMemo(QString safeContact, QString cid, QString zaddr,  int version=0, int headerNumber=1)
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
    h["c"] = safeContact;       // Is this a safe Contact request?
    j.setObject(h);
    header = j.toJson();
    qDebug() << "made header=" << header;
    return header;
}

// Create a Tx from the current state of the Chat page. 
Tx MainWindow::createTxFromChatPage() 
{
    Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
    {//scopeblock
        QString addr = ui->ContactZaddr->text().trimmed(); // We need to set the reply Address for our Contact here
        // Remove label if it exists
        addr = AddressBook::addressFromAddressLabel(addr);
        QString amtStr = "0";
        //  bool ok;
        CAmount amt = CAmount::fromDecimalString("0");
        totalAmt += amt;

  
        for(auto &c : AddressBook::getInstance()->getAllAddressLabels())
        {
            if (ui->ContactZaddr->text().trimmed() == c.getName()) 
            {
                QString cid = c.getCid();
                QString myAddr = c.getMyAddress();
                QString safeContact = "false";
                QString addr = c.getPartnerAddress();
                QString hmemo= createHeaderMemo(safeContact, cid, myAddr);
                QString memo = ui->memoTxtChat->toPlainText().trimmed();
                // ui->memoSizeChat->setLenDisplayLabel();
                tx.toAddrs.push_back(ToFields{addr, amt, hmemo}) ;
                qDebug() << hmemo;
                tx.toAddrs.push_back( ToFields{addr, amt, memo});
                qDebug() << "pushback chattx";
            }
        }
    }//end of scopeblock

    tx.fee = Settings::getMinerFee();
    qDebug() << "ChatTx created";
    return tx; 
}

void MainWindow::sendChatButton()
{
    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.

    // Memos can only be used with zAddrs. So check that first
    //auto addr = "zs1fllv4hgrjddnz2yz5dng9kchcg3wkhs0j2v5v3nc89w3r3kntkgq2sefcz2a9k2ycc8f6t0gm2q";
    //if (! Settings::isZAddress(AddressBook::addressFromAddressLabel(addr->text()))) {
        //QMessageBox msg(QMessageBox::Critical, tr("Memos can only be used with z-addresses"),
        //tr("The memo field can only be used with a z-address.\n") + addr->text() + tr("\ndoesn't look like a z-address"),
        //QMessageBox::Ok, this);
        //msg.exec();
        //return;
    //}

    Tx tx = createTxFromChatPage();
    QString error = doSendChatTxValidations(tx);
    if (!error.isEmpty()) 
    {
        // Something went wrong, so show an error and exit
        QMessageBox msg(
            QMessageBox::Critical, 
            tr("Message Error"),
            error,
            QMessageBox::Ok, 
            this
        );
        msg.exec();
        // abort the Tx
        qDebug() << "Tx aborted";
        return;
    }

        // Create a new Dialog to show that we are computing/sending the Tx
    auto d = new QDialog(this);
    auto connD = new Ui_ConnectionDialog();
    connD->setupUi(d);
    QPixmap logo(":/img/res/logobig.gif");
    connD->topIcon->setBasePixmap(
        logo.scaled(
            256, 
            256, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation
        )
    );
    connD->status->setText(tr("Please wait..."));
    connD->statusDetail->setText(tr("Your Message will be send"));
    d->show();
    // And send the Tx
    rpc->executeTransaction(tx, [=] (QString txid) { 
        ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
        connD->status->setText(tr("Done!"));
        connD->statusDetail->setText(txid);
        QTimer::singleShot(1000, [=] () {
            d->accept();
            d->close();
            delete connD;
            delete d;
                    
        });        
        // Force a UI update so we get the unconfirmed Tx
        rpc->refresh(true);

    }, 
    [=] (QString opid, QString errStr) {
    // Errored out
        ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
        d->accept();
        d->close();
        delete connD;
        delete d;
        if (!opid.isEmpty())
            errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            
                    
        QMessageBox::critical(this, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);            
    });
}        


QString MainWindow::doSendChatTxValidations(Tx tx) 
{
    // Check to see if we have enough verified funds to send the Tx.
    CAmount total;
    for (auto toAddr : tx.toAddrs) 
    {
        if (!Settings::isValidAddress(toAddr.addr)) 
        {
            QString addr = (toAddr.addr.length() > 100 ? toAddr.addr.left(100) + "..." : toAddr.addr);
            return QString(tr("Recipient Address ")) % addr % tr(" is Invalid");
        }

        // This technically shouldn't be possible, but issue #62 seems to have discovered a bug
        // somewhere, so just add a check to make sure. 
        if (toAddr.amount.toqint64() < 0)
            return QString(tr("Amount for address '%1' is invalid!").arg(toAddr.addr));

        total += toAddr.amount;
    }

    total += tx.fee;
    auto available = rpc->getModel()->getAvailableBalance();
    if (available < total) 
    {
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 5 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}

// Create a Safe Contact Request. 
Tx MainWindow::createTxForSafeContactRequest() 
{
    Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
    {//blockscope
        QString addr = ui->ContactZaddr->text().trimmed(); // We need to set the reply Address for our Contact here
        // Remove label if it exists
        addr = AddressBook::addressFromAddressLabel(addr);
        QString amtStr = "0";
        //bool ok;
        CAmount amt = CAmount::fromDecimalString("0");
        totalAmt += amt;
        for(auto &c : AddressBook::getInstance()->getAllAddressLabels())
        {
            if (ui->ContactZaddr->text().trimmed() == c.getName()) 
            {
                //QString cid = c.getCid();            // This has to be a new cid for the contact
                //QString myAddr = c.getMyAddress();   //  this should be a new HushChat zaddr
                //QString addr = c.getPartnerAddress(); //  this address will be insert by the user
                QString cid = "";
                QString myAddr = "";
                QString addr = "";
                QString safeContact = "true";
                QString hmemo= createHeaderMemo(safeContact,cid,myAddr);
                QString memo = ui->memoTxtChat->toPlainText().trimmed();
                //ui->memoSizeChat->setLenDisplayLabel();
                tx.toAddrs.push_back(ToFields{addr, amt, hmemo}) ;
                qDebug()<< hmemo;
                //tx.toAddrs.push_back( ToFields{addr, amt, memo});
                qDebug() << "pushback chattx";
            } 
        }
    }//end of blockscope

    tx.fee = Settings::getMinerFee();
    qDebug() << "ChatTx created";
    return tx;
}

void MainWindow::safeContactRequest() 
{
    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.
    // Memos can only be used with zAddrs. So check that first
    //auto addr = "zs1fllv4hgrjddnz2yz5dng9kchcg3wkhs0j2v5v3nc89w3r3kntkgq2sefcz2a9k2ycc8f6t0gm2q";
    //if (! Settings::isZAddress(AddressBook::addressFromAddressLabel(addr->text()))) {
        //QMessageBox msg(QMessageBox::Critical, tr("Memos can only be used with z-addresses"),
        //tr("The memo field can only be used with a z-address.\n") + addr->text() + tr("\ndoesn't look like a z-address"),
        //QMessageBox::Ok, this);
        //msg.exec();
        //return;
    //}
    Tx tx = createTxForSafeContactRequest();
    QString error = doSendChatTxValidations(tx);
    if (!error.isEmpty()) 
    {
        // Something went wrong, so show an error and exit
        QMessageBox msg(
            QMessageBox::Critical, 
            tr("Message Error"), 
            error,
            QMessageBox::Ok, 
            this
        );
        msg.exec();
        // abort the Tx
        qDebug() << "Tx aborted";
        return;
        
    }

    // Create a new Dialog to show that we are computing/sending the Tx
    auto d = new QDialog(this);
    auto connD = new Ui_ConnectionDialog();
    connD->setupUi(d);
    QPixmap logo(":/img/res/logobig.gif");
    connD->topIcon->setBasePixmap(logo.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    connD->status->setText(tr("Please wait..."));
    connD->statusDetail->setText(tr("Your Safe Contact Request will be send"));
    d->show();
    // And send the Tx
    rpc->executeTransaction(tx, [=] (QString txid) { 
        ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
        connD->status->setText(tr("Done!"));
        connD->statusDetail->setText(txid);
        QTimer::singleShot(1000, [=] () {
            d->accept();
            d->close();
            delete connD;
            delete d;
        });
        // Force a UI update so we get the unconfirmed Tx
        rpc->refresh(true);
    }, 
    [=] (QString opid, QString errStr) {
        // Errored out
        ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
        d->accept();
        d->close();
        delete connD;
        delete d;
        if (!opid.isEmpty())
            errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            

        QMessageBox::critical(
            this, 
            QObject::tr("Transaction Error"), 
            errStr, 
            QMessageBox::Ok
        );            
    });
}        


QString MainWindow::doSendRequestTxValidations(Tx tx) 
{
    // Check to see if we have enough verified funds to send the Tx.
    CAmount total;
    for (auto toAddr : tx.toAddrs) 
    {
        if (!Settings::isValidAddress(toAddr.addr)) 
        {
            QString addr = (toAddr.addr.length() > 100 ? toAddr.addr.left(100) + "..." : toAddr.addr);
            return QString(tr("Recipient Address ")) % addr % tr(" is Invalid");
        }

        // This technically shouldn't be possible, but issue #62 seems to have discovered a bug
        // somewhere, so just add a check to make sure. 
        if (toAddr.amount.toqint64() < 0) 
            return QString(tr("Amount for address '%1' is invalid!").arg(toAddr.addr));

        total += toAddr.amount;
    }

    total += tx.fee;
    auto available = rpc->getModel()->getAvailableBalance();
    if (available < total) 
    {
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 5 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}