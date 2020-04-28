#include "chatmodel.h"
#include "settings.h"
#include "ui_confirm.h"
#include "controller.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addressbook.h"
#include "ui_memodialog.h"



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
    /*for(auto &c : this->chatItems)
    {
        //view.getItems().add(QString("[Timestamp] <Contactname|Me>: lorem ipsum ...."));
    }*/
    qDebug() << "not implemented yet";
    //todo render items to view
}

void ChatModel::renderChatBox(QListWidget *view)
{
    qDebug() << "called ChatModel::renderChatBox(QListWidget *view)";
    QString line = "";
    while(view->count() > 0)
    {
        view->takeItem(0);
    }

    for(auto &c : this->chatItems)
    {
        QDateTime myDateTime;
 
        myDateTime.setTime_t(c.second.getTimestamp());
        qDebug() << "[" << myDateTime.toString("dd.MM.yyyy hh:mm:ss ") << "] " << "<" << c.second.getAddress() << "> :" << c.second.getMemo(); 
        line += QString("[") + myDateTime.toString("dd.MM.yyyy hh:mm:ss ") +  QString("] ");
        line += QString("<") + QString(c.second.getContact()) + QString("> :\n");
        line += QString(c.second.getMemo()) + QString("\n");
        view->addItem(line);
        line ="";
    }
}

void MainWindow::setupchatTab() {
       
// Send button
    QObject::connect(ui->sendChatButton, &QPushButton::clicked, this, &MainWindow::sendChatButton);

    }

ChatMemoEdit::ChatMemoEdit(QWidget* parent) : QPlainTextEdit(parent) {
    QObject::connect(this, &QPlainTextEdit::textChanged, this, &ChatMemoEdit::updateDisplay);
}

void ChatMemoEdit::updateDisplay() {
    QString txt = this->toPlainText();
    if (lenDisplayLabel)
        lenDisplayLabel->setText(QString::number(txt.toUtf8().size()) + "/" + QString::number(maxlen));

    if (txt.toUtf8().size() <= maxlen) {
        // Everything is fine
        if (sendChatButton)
            sendChatButton->setEnabled(true);

        if (lenDisplayLabel)
            lenDisplayLabel->setStyleSheet("");
    }
    else {
        // Overweight
        if (sendChatButton)
            sendChatButton->setEnabled(false);

        if (lenDisplayLabel)
            lenDisplayLabel->setStyleSheet("color: red;");
    }
}

void ChatMemoEdit::setMaxLen(int len) {
    this->maxlen = len;
    updateDisplay();
}

void ChatMemoEdit::setLenDisplayLabel(QLabel* label) {
    this->lenDisplayLabel = label;
}

void ChatMemoEdit::setSendChatButton(QPushButton* button) {
    this->sendChatButton = button;
}

// Create a Tx from the current state of the Chat page. 
Tx MainWindow::createTxFromChatPage() {
   Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
  {
        QString addr = ""; // We need to set the reply Address for our Contact here
        // Remove label if it exists
        addr = AddressBook::addressFromAddressLabel(addr);
        
        QString amtStr = "0";
      
      //  bool ok;
        CAmount amt;  

         
            amt = CAmount::fromDecimalString("0.00001");
            totalAmt = totalAmt + amt;
       

        QString memo = ui->memoTxtChat->toPlainText().trimmed();
        //ui->chatmemoSize->setLenDisplayLabel()
        
       
     tx.toAddrs.push_back(ToFields{addr, amt, memo,}) ;

         qDebug() << "pushback chattx";
   } 

    tx.fee = Settings::getMinerFee();

     return tx;

     qDebug() << "ChatTx created";
}

void MainWindow::sendChatButton() {
    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.

    // Memos can only be used with zAddrs. So check that first
   //auto addr = "zs1fllv4hgrjddnz2yz5dng9kchcg3wkhs0j2v5v3nc89w3r3kntkgq2sefcz2a9k2ycc8f6t0gm2q";
   // if (! Settings::isZAddress(AddressBook::addressFromAddressLabel(addr->text()))) {
     //   QMessageBox msg(QMessageBox::Critical, tr("Memos can only be used with z-addresses"),
       // tr("The memo field can only be used with a z-address.\n") + addr->text() + tr("\ndoesn't look like a z-address"),
       // QMessageBox::Ok, this);

       // msg.exec();
        //return;
    //}

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
        QPixmap logo(":/img/res/logobig.gif");
        connD->topIcon->setBasePixmap(logo.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        connD->status->setText(tr("Please wait..."));
        connD->statusDetail->setText(tr("Your Message will be send"));

        d->show();

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
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 5 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}