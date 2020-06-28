// Copyright 2019-2020 The Hush developers
// GPLv3

#include "addressbook.h"
#include "ui_addressbook.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mainwindow.h"
#include "controller.h"
#include "DataStore/DataStore.h"
#include "FileSystem/FileSystem.h"


AddressBookModel::AddressBookModel(QTableView *parent) : QAbstractTableModel(parent) 
{
    headers << tr("Avatar")<< tr("Label") << tr("Address") << tr("HushChatAddress") << tr("CID");
    this->parent = parent;
    loadData();
}

AddressBookModel::~AddressBookModel() 
{
    saveData();
}

void AddressBookModel::saveData() 
{
    // Save column positions
    QSettings().setValue(
        "addresstablegeometry",
        parent->horizontalHeader()->saveState()
    );
}


void AddressBookModel::loadData() 
{        
    labels = AddressBook::getInstance()->getAllAddressLabels();
    parent->horizontalHeader()->restoreState(
        QSettings().value(
            "addresstablegeometry"
        ).toByteArray()      
    );
     
}

void AddressBookModel::addNewLabel(QString label, QString addr, QString myAddr, QString cid, QString avatar) 
{
    //labels.push_back(QPair<QString, QString>(label, addr));
    AddressBook::getInstance()->addAddressLabel(label, addr, myAddr, cid, avatar);
    updateUi();
    
}

void AddressBookModel::updateUi()
{
    labels.clear();
    labels = AddressBook::getInstance()->getAllAddressLabels();
    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

void AddressBookModel::removeItemAt(int row) 
{
    if (row >= labels.size())
        return;

    AddressBook::getInstance()->removeAddressLabel(labels[row].getName(), labels[row].getPartnerAddress(), labels[row].getMyAddress(),labels[row].getCid(),labels[row].getAvatar());    
    labels.clear();
    labels = AddressBook::getInstance()->getAllAddressLabels();
    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

ContactItem AddressBookModel::itemAt(int row) 
{
    if (row >= labels.size())
    {
        ContactItem item = ContactItem("", "", "", "","");
        return item;
    }   
        

    return labels.at(row);
}



int AddressBookModel::rowCount(const QModelIndex&) const 
{
    return labels.size();
}

int AddressBookModel::columnCount(const QModelIndex&) const 
{
    return headers.size();
}


QVariant AddressBookModel::data(const QModelIndex &index, int role) const 
{
    if (role == Qt::DisplayRole) 
    {
        switch(index.column()) 
        {
            case 0: return labels.at(index.row()).getAvatar();
            case 1: return labels.at(index.row()).getName();
            case 2: return labels.at(index.row()).getPartnerAddress();
            case 3: return labels.at(index.row()).getMyAddress();
            case 4: return labels.at(index.row()).getCid();

        }
        
    }

    return QVariant();
}  


QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const 
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) 
        return headers.at(section);

    return QVariant();
}


//===============
// AddressBook
//===============
void AddressBook::open(MainWindow* parent, QLineEdit* target) 
{
    QDialog d(parent);
    Ui_addressBook ab;
    ab.setupUi(&d);
    Settings::saveRestore(&d);
    QRegExpValidator v(QRegExp(Settings::labelRegExp), ab.label);
    ab.label->setValidator(&v);
    AddressBookModel model(ab.addresses);
    ab.addresses->setModel(&model);

    // If there is no target, the we'll call the button "Ok", else "Pick"
    if (target != nullptr) 
        ab.buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Pick"));

    // Connect the dialog's closing to updating the label address completor
    QObject::connect(&d, &QDialog::finished, [=] (auto) { parent->updateLabels(); });

    Controller* rpc = parent->getRPC();
    QObject::connect(ab.newZaddr, &QPushButton::clicked, [&] () { 
       
        bool sapling = true;
        try 
       {
        rpc->createNewZaddr(sapling, [=] (QJsonValue reply) {
            QString myAddr = reply.toArray()[0].toString();
            QString message = QString("New Chat Address for your partner: ") + myAddr;
            QString cid = QUuid::createUuid().toString(QUuid::WithoutBraces);
 
            rpc->refreshAddresses();
           
            parent->ui->listReceiveAddresses->insertItem(0, myAddr); 
            parent->ui->listReceiveAddresses->setCurrentIndex(0);

            qDebug() << " new Addr in Addressbook" << myAddr;
            ab.cid->setText(cid);
            ab.addr_chat->setText(myAddr);
        });
       }

       catch(...)
       {

            
            qDebug() << QString("Caught something nasty with myZaddr Addressbook");
       }
    });
   
       // model.updateUi(); //todo fix updating gui after adding 

    // If there is a target then make it the addr for the "Add to" button
    if (target != nullptr && Settings::isValidAddress(target->text())) 
    {
        ab.addr->setText(target->text());
        ab.label->setFocus();
    }

    // Add new address button
    QObject::connect(ab.addNew, &QPushButton::clicked, [&] () {
        auto addr = ab.addr->text().trimmed();
        auto myAddr = ab.addr_chat->text().trimmed();
        QString newLabel = ab.label->text();
        QString cid = ab.cid->text();
        
        
        QString avatar = QString(":/icons/res/") + ab.comboBoxAvatar->currentText() + QString(".png");

        if (addr.isEmpty() || newLabel.isEmpty()) 
        {
            QMessageBox::critical(
                parent, 
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
                parent, 
                QObject::tr("Address Format Error"), 
                QObject::tr("%1 doesn't seem to be a valid hush address.").arg(addr), 
                QMessageBox::Ok
            );
            return;
        } 

        // Don't allow duplicate address labels.                 
        if (!getInstance()->getAddressForLabel(newLabel).isEmpty()) 
        {
            QMessageBox::critical(
                parent, 
                QObject::tr("Label Error"), 
                QObject::tr("The label '%1' already exists. Please remove the existing label.").arg(newLabel), 
                QMessageBox::Ok
            );
            return;
        } 

        ////// We need a better popup here. 
            AddressBook::getInstance()->addAddressLabel(newLabel, addr, myAddr, cid,avatar);

             QMessageBox::information(
                parent, 
                QObject::tr("Added Contact"), 
                QObject::tr("successfully added your new contact").arg(newLabel), 
                QMessageBox::Ok
            );
            return;
        
        
       // rpc->refresh(true);
         model.updateUi();

        rpc->refreshContacts(
        parent->ui->listContactWidget
            
        );

       parent->ui->listChat->verticalScrollBar()->setValue(
        parent->ui->listChat->verticalScrollBar()->maximum());

    });

   //  AddressBook::getInstance()->addAddressLabel(newLabel, ab.addr->text(), cid);

    // Import Button
    QObject::connect(ab.btnImport, &QPushButton::clicked, [&] () {
        // Get the import file name.
        auto fileName = QFileDialog::getOpenFileUrl(
            &d, QObject::tr("Import Address Book"), 
            QUrl(), 
            "CSV file (*.csv)"
            );
        if (fileName.isEmpty())
            return;

        QFile file(fileName.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) 
        {
            QMessageBox::information(
                &d, 
                QObject::tr("Unable to open file"),
                file.errorString()
            );
            return;
        }        

        QTextStream in(&file);
        QString line;
        int numImported = 0;
        while (in.readLineInto(&line)) 
        {
            QStringList items = line.split(",");
            if (items.size() != 2)
                continue;

            if (!Settings::isValidAddress(items.at(0)))
                continue;

            // Add label, address.
            model.addNewLabel(items.at(1), items.at(0), "", "", "");
            numImported++;
        }

        QMessageBox::information(
            &d,
            QObject::tr("Address Book Import Done"),
            QObject::tr("Imported %1 new Address book entries").arg(numImported)
        );
    });

    auto fnSetTargetLabelAddr = [=] (QLineEdit* target, QString label, QString addr, QString myAddr, QString cid, QString avatar) {
        target->setText(label % "/" % addr   % myAddr);
    };

    // Double-Click picks the item
    QObject::connect(ab.addresses, &QTableView::doubleClicked, [&] (auto index) {
        // If there's no target, then double-clicking does nothing.
        if (!target)
            return;

        if (index.row() < 0)
            return;

        QString lbl  = model.itemAt(index.row()).getName();
        QString addr = model.itemAt(index.row()).getPartnerAddress();
        QString myAddr = model.itemAt(index.row()).getMyAddress();
        QString cid = model.itemAt(index.row()).getCid();
        QString avatar = model.itemAt(index.row()).getCid();
        d.accept();
        fnSetTargetLabelAddr(target, lbl, addr, myAddr, cid, avatar);
    });

    // Right-Click
    ab.addresses->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ab.addresses, &QTableView::customContextMenuRequested, [&] (QPoint pos) {
        QModelIndex index = ab.addresses->indexAt(pos);
        if (index.row() < 0)
            return;

        QString lbl  = model.itemAt(index.row()).getName();
        QString addr = model.itemAt(index.row()).getPartnerAddress();
        QString myAddr = model.itemAt(index.row()).getMyAddress();
        QString cid = model.itemAt(index.row()).getCid();
        QString avatar = model.itemAt(index.row()).getAvatar();

        QMenu menu(parent);

        if (target != nullptr)
            menu.addAction("Pick", [&] () {
                d.accept();
                fnSetTargetLabelAddr(target, lbl, addr, myAddr, cid, avatar);
            });

        menu.addAction(QObject::tr("Copy address"), [&] () {
            QGuiApplication::clipboard()->setText(addr);            
            parent->ui->statusBar->showMessage(QObject::tr("Copied to clipboard"), 3 * 1000);
        });

        menu.addAction(QObject::tr("Delete label"), [&] () {
            model.removeItemAt(index.row());
        });

        menu.exec(ab.addresses->viewport()->mapToGlobal(pos));    
    });

    if (d.exec() == QDialog::Accepted && target != nullptr) {
        auto selection = ab.addresses->selectionModel();
        if (selection && selection->hasSelection() && selection->selectedRows().size() > 0) {
            auto item = model.itemAt(selection->selectedRows().at(0).row());
            fnSetTargetLabelAddr(target, item.getName(), item.getMyAddress(), item.getPartnerAddress(),  item.getCid(), item.getAvatar());
        }
    };

    // Refresh after the dialog is closed to update the labels everywhere.
    parent->getRPC()->refresh(true);
    model.updateUi(); //todo fix updating gui after adding 
}

//=============
// AddressBook singleton class
//=============
AddressBook* AddressBook::getInstance() 
{
    if (!instance)
        instance = new AddressBook();

    return instance;
}

AddressBook::AddressBook() 
{
    readFromStorage();
}

void AddressBook::readFromStorage() 
{
    auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QString target_decaddr_file = dir.filePath("addresslabels.dat");
    QString target_encaddr_file = dir.filePath("addresslabels.dat.enc");
    QFile file(target_encaddr_file);
    QFile file1(target_decaddr_file);

    if (file.exists()) 
    {

        // Decrypt first 

        QString passphraseHash = DataStore::getChatDataStore()->getPassword();
        int length = passphraseHash.length();

        char *sequence1 = NULL;
        sequence1 = new char[length+1];
        strncpy(sequence1, passphraseHash.toUtf8(), length+1);

        #define PassphraseHashEnd ((const unsigned char *) sequence1)
        #define MESSAGE_LEN length

        #define PASSWORD sequence
        #define KEY_LEN crypto_box_SEEDBYTES

        const QByteArray ba = QByteArray::fromHex(passphraseHash.toLatin1());
        const unsigned char *pwHash= reinterpret_cast<const unsigned char *>(ba.constData());
 

        

        FileEncryption::decrypt(target_decaddr_file, target_encaddr_file, pwHash);


        allLabels.clear();
        file1.open(QIODevice::ReadOnly);
        QDataStream in(&file1);    // read the data serialized from the file
        QString version;
        in >> version;
        QList<QList<QString>> stuff;
        in >> stuff;

            //////////////found old addrbook, and rename it to .bak
        if (version == "v1")
        {
            auto filename = QStringLiteral("addresslabels.dat");
            auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
            QFile address(dir.filePath(filename));

            qDebug() << "is v1";

            address.rename(dir.filePath("addresslabels.bak"));
            
        }else{
        for (int i=0; i < stuff.size(); i++) 
        {

            ContactItem contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3],stuff[i][4]);

            allLabels.push_back(contact);
        }
        }
   
 
      //  qDebug() << "Read " << version << " Hush contacts from disk...";
        file1.close();

        FileEncryption::encrypt(target_encaddr_file, target_decaddr_file, pwHash);
        file1.remove();
    }
    else 
    {
        qDebug() << "No Hush contacts found on disk!";
    }

}


void AddressBook::writeToStorage() 
{
    //FileSystem::getInstance()->writeContacts(AddressBook::writeableFile(), DataStore::getContactDataStore()->dump());
    
   // FileSystem::getInstance()->writeContactsOldFormat(AddressBook::writeableFile(), allLabels);

        QString passphraseHash = DataStore::getChatDataStore()->getPassword();
        int length = passphraseHash.length();

        char *sequence1 = NULL;
        sequence1 = new char[length+1];
        strncpy(sequence1, passphraseHash.toUtf8(), length+1);

        #define PassphraseHashEnd ((const unsigned char *) sequence1)
        #define MESSAGE_LEN length

        #define PASSWORD sequence
        #define KEY_LEN crypto_box_SEEDBYTES

        const QByteArray ba = QByteArray::fromHex(passphraseHash.toLatin1());
        const unsigned char *pwHash= reinterpret_cast<const unsigned char *>(ba.constData());

  
    
        auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QString target_encaddr_file = dir.filePath("addresslabels.dat.enc");
        QString target_decaddr_file = dir.filePath("addresslabels.dat");

        FileEncryption::decrypt(target_decaddr_file, target_encaddr_file, pwHash);

    QFile file(target_decaddr_file);
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&file);   // we will serialize the data into the file
    QList<QList<QString>> contacts;

    for(auto &item: allLabels)
    {
        QList<QString> c;
        c.push_back(item.getName());
        c.push_back(item.getPartnerAddress());
        c.push_back(item.getMyAddress());
        c.push_back(item.getCid());
        c.push_back(item.getAvatar());
        contacts.push_back(c);   
        
    }
    out << QString("v2") << contacts;
    qDebug()<<"schreibe in Datei: ";
    file.close();


        FileEncryption::encrypt(target_encaddr_file, target_decaddr_file , pwHash);
        QFile file1(target_decaddr_file);
        file1.remove();
    
        qDebug()<<"encrypt Addrbook writeToStorage";
    
}

QString AddressBook::writeableFile() 
{
    auto filename = QStringLiteral("addresslabels.dat");
    auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkpath(dir.absolutePath());

    if (Settings::getInstance()->isTestnet())
        return dir.filePath("testnet-" % filename);

    else
        return dir.filePath(filename);
}


// Add a new address/label to the database
void AddressBook::addAddressLabel(QString label, QString address, QString myAddr, QString cid, QString avatar) 
{
    Q_ASSERT(Settings::isValidAddress(address));
    // getName(), remove any existing label
    // Iterate over the list and remove the label/address
    for (int i=0; i < allLabels.size(); i++)
        if (allLabels[i].getName() == label)
            removeAddressLabel(allLabels[i].getName(), allLabels[i].getPartnerAddress(),allLabels[i].getMyAddress(), allLabels[i].getCid(), allLabels[i].getAvatar());

    ContactItem item = ContactItem(label, address, myAddr, cid, avatar);
    allLabels.push_back(item);
    writeToStorage();
}

// Remove a new address/label from the database
void AddressBook::removeAddressLabel(QString label, QString address, QString myAddr, QString cid, QString avatar) 
{
    // Iterate over the list and remove the label/address
    for (int i=0; i < allLabels.size(); i++) 
    {
        if (allLabels[i].getName() == label && allLabels[i].getPartnerAddress() == address) 
        {
            allLabels.removeAt(i);
            writeToStorage();
            return;
        }
    }
}

void AddressBook::updateLabel(QString oldlabel, QString address, QString newlabel) 
{
    // Iterate over the list and update the label/address
    for (int i = 0; i < allLabels.size(); i++) 
    {
        if (allLabels[i].getName() == oldlabel && allLabels[i].getPartnerAddress() == address) 
        {
            allLabels[i].setName(newlabel);
            writeToStorage();
            return;
        }
    }
}

// Read all addresses
const QList<ContactItem>& AddressBook::getAllAddressLabels() 
{
    if (allLabels.isEmpty())
        readFromStorage();

    return allLabels;
}

// Get the label for an address
QString AddressBook::getLabelForAddress(QString addr) 
{
    for (auto i : allLabels)
        if (i.getPartnerAddress() == addr)
            return i.getName();

    return "";
}

// Get the address for a label
QString AddressBook::getAddressForLabel(QString label) 
{
    for (auto i: allLabels)
        if (i.getName() == label)
            return i.getPartnerAddress();

    return "";
}

QString AddressBook::addLabelToAddress(QString addr) 
{
    QString label = AddressBook::getInstance()->getLabelForAddress(addr);
    if (!label.isEmpty())
        return label + "/" + addr;

    else
        return addr;
}



QString AddressBook::addressFromAddressLabel(const QString& lblAddr) 
{ 
    return lblAddr.trimmed().split("/").last(); 
}

AddressBook* AddressBook::instance = nullptr;
