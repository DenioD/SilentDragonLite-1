#include "addressbook.h"
#include "ui_addressbook.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mainwindow.h"
#include "controller.h"


AddressBookModel::AddressBookModel(QTableView *parent) : QAbstractTableModel(parent) 
{
    headers << tr("Label") << tr("Address") << tr("HushChatAddress") << tr("cid");
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

void AddressBookModel::addNewLabel(QString label, QString addr, QString myAddr, QString cid) 
{
    //labels.push_back(QPair<QString, QString>(label, addr));
    AddressBook::getInstance()->addAddressLabel(label, addr, myAddr, cid);
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

    AddressBook::getInstance()->removeAddressLabel(labels[row].getName(), labels[row].getPartnerAddress(), labels[row].getMyAddress(),labels[row].getCid());    
    labels.clear();
    labels = AddressBook::getInstance()->getAllAddressLabels();
    dataChanged(index(0, 0), index(labels.size()-1, columnCount(index(0,0))-1));
    layoutChanged();
}

ContactItem AddressBookModel::itemAt(int row) 
{
    if (row >= labels.size())
    {
        ContactItem item = ContactItem("", "", "", "");
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
            case 0: return labels.at(index.row()).getName();
            case 1: return labels.at(index.row()).getPartnerAddress();
            case 2: return labels.at(index.row()).getMyAddress();
            case 3: return labels.at(index.row()).getCid();
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
        bool sapling = true;
        rpc->createNewZaddr(sapling, [=] (json reply) {
            QString myAddr = QString::fromStdString(reply.get<json::array_t>()[0]);
         // QString myAddr = "zs1flslqurcnummsw37mfxkhx6d7uwpevlc78cdjyqrgng7357t8f3stm9fneeqtuupfnrt7f933a9";
            QString message = QString("New Chat Address for your partner: ") + myAddr;
           
            parent->ui->listReceiveAddresses->insertItem(0, myAddr); 
            parent->ui->listReceiveAddresses->setCurrentIndex(0);
           // ab.addr_chat->setText(myAddr);
            qDebug() << "new generated myAddr" << myAddr;
             // 
            ab.addr_chat->setText(myAddr);
           // AddressBook::getInstance()->addAddressLabel(newLabel, ab.addr->text(), myAddr, cid);
        });
        model.updateUi(); //todo fix updating gui after adding 
        rpc->refresh(true);

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
        QString cid = QUuid::createUuid().toString(QUuid::WithoutBraces);

        

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
            AddressBook::getInstance()->addAddressLabel(newLabel, addr, myAddr, cid);
            QMessageBox::critical(
                parent, 
                QObject::tr("Add Successfully"), 
                QObject::tr("juhu").arg(newLabel), 
                QMessageBox::Ok
            );
            return;
        
        
        rpc->refresh(true);
         model.updateUi();

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
            model.addNewLabel(items.at(1), items.at(0), "", "");
            numImported++;
        }

        QMessageBox::information(
            &d,
            QObject::tr("Address Book Import Done"),
            QObject::tr("Imported %1 new Address book entries").arg(numImported)
        );
    });

    auto fnSetTargetLabelAddr = [=] (QLineEdit* target, QString label, QString addr, QString myAddr, QString cid) {
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
        d.accept();
        fnSetTargetLabelAddr(target, lbl, addr, myAddr, cid);
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

        QMenu menu(parent);

        if (target != nullptr)
            menu.addAction("Pick", [&] () {
                d.accept();
                fnSetTargetLabelAddr(target, lbl, addr, myAddr, cid);
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
            fnSetTargetLabelAddr(target, item.getName(), item.getMyAddress(), item.getPartnerAddress(),  item.getCid());
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
    QFile file(AddressBook::writeableFile());

    if (file.exists()) 
    {
        allLabels.clear();
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);    // read the data serialized from the file
        QString version;
        in >> version;
        qDebug() << "Detected old addressbook format";
            // Convert old addressbook format v1 to v2
        QList<QList<QString>> stuff;
        in >> stuff;
        //qDebug() << "Stuff: " << stuff;
        for (int i=0; i < stuff.size(); i++) 
        {
            //qDebug() << "0:" << stuff[i][0];
            //qDebug() << "1:" << stuff[i][1];
            //qDebug() << "2:" << stuff[i][2];
            ContactItem contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3]);
            //qDebug() << "contact=" << contact.toQTString();
            allLabels.push_back(contact);
        }

        qDebug() << "Read " << version << " Hush contacts from disk...";
        file.close();
    }
    else 
    {
        qDebug() << "No Hush contacts found on disk!";
    }

    // Special. 
    // Add the default silentdragon donation address if it isn't already present
    // QList<QString> allAddresses;
    // std::transform(allLabels.begin(), allLabels.end(), 
    //     std::back_inserter(allAddresses), [=] (auto i) { return i.getPartnerAddress(); });
    // if (!allAddresses.contains(Settings::getDonationAddr(true))) {
    //     allLabels.append(QPair<QString, QString>("silentdragon donation", Settings::getDonationAddr(true)));
    // }
}

void AddressBook::writeToStorage() 
{
    QFile file(AddressBook::writeableFile());
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
        contacts.push_back(c);
    }
    out << QString("v1") << contacts;
    file.close();
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
void AddressBook::addAddressLabel(QString label, QString address, QString myAddr, QString cid) 
{
    Q_ASSERT(Settings::isValidAddress(address));
    // getName(), remove any existing label
    // Iterate over the list and remove the label/address
    for (int i=0; i < allLabels.size(); i++)
        if (allLabels[i].getName() == label)
            removeAddressLabel(allLabels[i].getName(), allLabels[i].getPartnerAddress(),allLabels[i].getMyAddress(), allLabels[i].getCid());

    ContactItem item = ContactItem(label, address, myAddr, cid);
    allLabels.push_back(item);
    writeToStorage();
}

// Remove a new address/label from the database
void AddressBook::removeAddressLabel(QString label, QString address, QString myAddr, QString cid) 
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
