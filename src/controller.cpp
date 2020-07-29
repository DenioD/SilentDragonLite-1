// Copyright 2019-2020 The Hush developers
// GPLv3

#include "controller.h"
#include "mainwindow.h"
#include "addressbook.h"
#include "settings.h"
#include "version.h"
#include "camount.h"
#include "websockets.h"
#include "Model/ChatItem.h"
#include "DataStore/DataStore.h"

ChatModel *chatModel = new ChatModel();
Chat *chat = new Chat();
ContactModel *contactModel = new ContactModel();

using json = nlohmann::json;


Controller::Controller(MainWindow* main) 
{
    auto cl = new ConnectionLoader(main, this);

    // Execute the load connection async, so we can set up the rest of RPC properly. 
    QTimer::singleShot(1, [=]() { cl->loadConnection(); });

    this->main = main;
    this->ui = main->ui;

    // Setup balances table model
    balancesTableModel = new BalancesTableModel(main->ui->balancesTable);
    main->ui->balancesTable->setModel(balancesTableModel);

    // Setup transactions table model
    transactionsTableModel = new TxTableModel(ui->transactionsTable);
    main->ui->transactionsTable->setModel(transactionsTableModel);
    
    // Set up timer to refresh Price
    priceTimer = new QTimer(main);
    QObject::connect(priceTimer, &QTimer::timeout, [=]() {
        if (Settings::getInstance()->getAllowFetchPrices()) 
            refreshZECPrice();          
        
    });
    priceTimer->start(Settings::priceRefreshSpeed);  // Every 5 Min

    // Set up a timer to refresh the UI every few seconds
    timer = new QTimer(main);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        refresh();
    });
    timer->start(Settings::updateSpeed);    

    // Create the data model
    model = new DataModel();

    // Crate the hushdRPC 
    zrpc = new LiteInterface();
}

Controller::~Controller() 
{
    delete timer;
    delete txTimer;
    delete transactionsTableModel;
    delete balancesTableModel;
    delete model;
    delete zrpc;
}
 
// Called when a connection to hushd is available. 
void Controller::setConnection(Connection* c) 
{
    if (c == nullptr) 
        return;

    this->zrpc->setConnection(c);
    ui->statusBar->showMessage("");

    // If we're allowed to get the Hush Price, get the prices
    if (Settings::getInstance()->getAllowFetchPrices())
        refreshZECPrice();

    // If we're allowed to check for updates, check for a new release
    if (Settings::getInstance()->getCheckForUpdates())
        checkForUpdate();

    // Force update, because this might be coming from a settings update
    // where we need to immediately refresh
    refresh(true);

    // Create Sietch zdust addr at startup.
    // Using DataStore singelton, to store the data outside of lambda, bing bada boom :D
    for(uint8_t i = 0; i < 8; i++)
    {
        zrpc->createNewSietchZaddr( [=] (json reply) {
            QString zdust = QString::fromStdString(reply.get<json::array_t>()[0]);
            DataStore::getSietchDataStore()->setData("Sietch" + QString(i), zdust.toUtf8());
        });
    }
       refreshContacts(
            ui->listContactWidget
            
        );

        ui->listChat->verticalScrollBar()->setValue(
        ui->listChat->verticalScrollBar()->maximum());
}

std::string Controller::encryptDecrypt(std::string toEncrypt) 
{ 

    int radomInteger = rand() % 1000000000 +100000;
    
     
    char key = radomInteger;
    std::string output = toEncrypt;
    
    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key;
    
    return output;

}

// Build the RPC JSON Parameters for this tx
void Controller::fillTxJsonParams(json& allRecepients, Tx tx)
{   
    Q_ASSERT(allRecepients.is_array());

    // Construct the JSON params
    json rec = json::object();

    //creating the JSON dust parameters in a std::vector to iterate over there during tx
    std::vector<json> dust(8);
    dust.resize(8, json::object());

    // Create Sietch zdust addr again to not use it twice.
    // Using DataStore singelton, to store the data outside of lambda, bing bada boom :D
    for(uint8_t i = 0; i < 8; i++)
    {
        zrpc->createNewSietchZaddr( [=] (json reply) {
            QString zdust = QString::fromStdString(reply.get<json::array_t>()[0]);
            DataStore::getSietchDataStore()->setData(QString("Sietch") + QString(i), zdust.toUtf8());
        } );
    }
    // Set sietch zdust addr to json.
    // Using DataStore singelton, to store the data into the dust.
    for(uint8_t i = 0; i < 8; i++)
    {
    dust.at(i)["address"] = DataStore::getSietchDataStore()->getData(QString("Sietch" + QString(i))).toStdString();
    }

    DataStore::getSietchDataStore()->clear(); // clears the datastore

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    int sizerandomString = rand() % 120 +10;
    const int randomStringLength = sizerandomString;

    QString randomString;
    QRandomGenerator *gen = QRandomGenerator::system();
    for(int i=0; i<randomStringLength; ++i)
    {
       // int index = qrand() % possibleCharacters.length();
       int index = gen->bounded(0, possibleCharacters.length() - 1);
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
    }
  
   for(uint8_t i = 0; i < 8; i++)
    {
        int length = randomString.length(); 
        int randomSize = rand() % 120 +10;
        char *randomHash = NULL;
        randomHash = new char[length+1];
        strncpy(randomHash, randomString.toLocal8Bit(), length +1);  
        #define MESSAGE ((const unsigned char *) randomHash)
        #define MESSAGE_LEN length
        #define MESSAGE_LEN1 length + randomSize
    
        unsigned char hash[crypto_secretstream_xchacha20poly1305_ABYTES];

        crypto_generichash(hash, sizeof hash,
                   MESSAGE, MESSAGE_LEN1,
                   NULL, 0);

        std::string decryptedMemo(reinterpret_cast<char*>(hash),MESSAGE_LEN1);
        std::string encrypt = this->encryptDecrypt(decryptedMemo);
        QString randomHashafter1 = QByteArray(reinterpret_cast<const char*>(encrypt.c_str()),encrypt.size()).toHex();
        dust.at(i)["memo"] = randomHashafter1.toStdString();
    }

    for(auto &it: dust)
    {
        it["amount"] = 0;
    }

        
    // For each addr/amt/memo, construct the JSON and also build the confirm dialog box   
    for (int i=0; i < tx.toAddrs.size(); i++) 
    {
        auto toAddr = tx.toAddrs[i];
        rec["address"] = toAddr.addr.toStdString();
        rec["amount"]  = toAddr.amount.toqint64();
        if (Settings::isZAddress(toAddr.addr) && !toAddr.memo.trimmed().isEmpty())
            rec["memo"] = toAddr.memo.toStdString();

        allRecepients.push_back(rec);
    }

    int decider = rand() % 100 + 1 ;  ; // random int between 1 and 100

if (tx.toAddrs.size() < 2)
{

if(decider % 4 == 3) 
{

    allRecepients.insert(std::begin(allRecepients), {
            dust.at(0),    
            dust.at(1),
            dust.at(2),
            dust.at(3),
            dust.at(4),
            dust.at(5)

        }) ;
      
}else{

 allRecepients.insert(std::begin(allRecepients), {
            dust.at(0),
            dust.at(1),
            dust.at(2),
            dust.at(3),
            dust.at(4),
            dust.at(5),
            dust.at(6)

        }) ;

}
}else{

if(decider % 4 == 3) 
{

    allRecepients.insert(std::begin(allRecepients), {
            dust.at(0),    
            dust.at(1),
            dust.at(2),
            dust.at(3),
            dust.at(4)


        }) ;
      
}else{

 allRecepients.insert(std::begin(allRecepients), {
            dust.at(0),
            dust.at(1),
            dust.at(2),
            dust.at(3),
            dust.at(4),
            dust.at(5)

        }) ;

}
}


}

void Controller::noConnection() 
{    
    QIcon i = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    main->statusIcon->setPixmap(i.pixmap(16, 16));
    main->statusIcon->setToolTip("");
    main->statusLabel->setText(QObject::tr("No Connection"));
    main->statusLabel->setToolTip("");
    main->ui->statusBar->showMessage(QObject::tr("No Connection"), 1000);

    // Clear balances table.
    QMap<QString, CAmount> emptyBalances;
    QList<UnspentOutput>  emptyOutputs;
    QList<QString>        emptyAddresses;
    balancesTableModel->setNewData(emptyAddresses, emptyAddresses, emptyBalances, emptyOutputs);

    // Clear Transactions table.
    QList<TransactionItem> emptyTxs;
    transactionsTableModel->replaceData(emptyTxs);

    // Clear balances
    ui->balSheilded->setText("");
    ui->balTransparent->setText("");
    ui->balTotal->setText("");

    ui->balSheilded->setToolTip("");
    ui->balTransparent->setToolTip("");
    ui->balTotal->setToolTip("");
}

/// This will refresh all the balance data from hushd
void Controller::refresh(bool force) 
{
    if (!zrpc->haveConnection()) 
        return noConnection();

    getInfoThenRefresh(force);
}

void Controller::processInfo(const json& info)
{
    // Testnet?
    QString chainName;
    if (!info["chain_name"].is_null())
    {
        chainName = QString::fromStdString(info["chain_name"].get<json::string_t>());
        Settings::getInstance()->setTestnet(chainName == "test");
    }

    QString version = QString::fromStdString(info["version"].get<json::string_t>());
    Settings::getInstance()->sethushdVersion(version);

    // Recurring pamynets are testnet only
    if (!Settings::getInstance()->isTestnet())
        main->disableRecurring();
}

void Controller::getInfoThenRefresh(bool force) 
{
    if (!zrpc->haveConnection()) 
        return noConnection();

    static bool prevCallSucceeded = false;

    zrpc->fetchInfo([=] (const json& reply) {
        prevCallSucceeded = true;       
        int curBlock  = reply["latest_block_height"].get<json::number_integer_t>();
        bool doUpdate = force || (model->getLatestBlock() != curBlock);
        int difficulty = reply["difficulty"].get<json::number_integer_t>();
        int blocks_until_halving= 340000 - curBlock;
        int halving_days = (blocks_until_halving * 150) / (60 * 60 * 24) ;
        int longestchain = reply["longestchain"].get<json::number_integer_t>();
        int notarized = reply["notarized"].get<json::number_integer_t>();
        
        model->setLatestBlock(curBlock);
        if (
            Settings::getInstance()->get_currency_name() == "EUR" || 
            Settings::getInstance()->get_currency_name() == "CHF" || 
            Settings::getInstance()->get_currency_name() == "RUB"
        ) 
        {
            ui->blockHeight->setText(
                "Block: " + QLocale(QLocale::German).toString(curBlock)
            );
            ui->last_notarized->setText(
                "Block: " + QLocale(QLocale::German).toString(notarized)
            );
            ui->longestchain->setText(
                "Block: " + QLocale(QLocale::German).toString(longestchain)
            );
            ui->difficulty->setText(
                QLocale(QLocale::German).toString(difficulty)
            );
            ui->halvingTime->setText(
                (QLocale(QLocale::German).toString(blocks_until_halving)) + 
                " Blocks or , " + (QLocale(QLocale::German).toString(halving_days)  + " days" )
            );
        }
        else 
        {
            ui->blockHeight->setText(
                "Block: " + QLocale(QLocale::English).toString(curBlock)
            );
            ui->last_notarized->setText(
                "Block: " + QLocale(QLocale::English).toString(notarized)
            );
            ui->longestchain->setText(
                "Block: " + QLocale(QLocale::English).toString(longestchain)
            );
            ui->difficulty->setText(
                QLocale(QLocale::English).toString(difficulty)
            );
            ui->halvingTime->setText(
                (QLocale(QLocale::English).toString(blocks_until_halving)) + 
                " Blocks or , " + (QLocale(QLocale::English).toString(halving_days)  + " days" )
            );
        }

        ui->Version->setText(QString::fromStdString(reply["version"].get<json::string_t>())); 
        ui->Vendor->setText(QString::fromStdString(reply["vendor"].get<json::string_t>()));

        main->logger->write(
            QString("Refresh. curblock ") % QString::number(curBlock) % ", update=" % (doUpdate ? "true" : "false") 
        );

        // Connected, so display checkmark.
        auto tooltip = Settings::getInstance()->getSettings().server + "\n" + 
                            QString::fromStdString(zrpc->getConnection()->getInfo().dump());
        QIcon i(":/icons/res/connected.gif");
        QString chainName = Settings::getInstance()->isTestnet() ? "test" : "main";
        main->statusLabel->setText(chainName + "(" + QString::number(curBlock) + ")");

        // use currency ComboBox as input 

        if (Settings::getInstance()->get_currency_name() == "USD") 
        {
            double price = Settings::getInstance()->getZECPrice();
            double volume = Settings::getInstance()->getUSDVolume();
            double cap =  Settings::getInstance()->getUSDCAP();
            main->statusLabel->setText(
                " HUSH/USD=$ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " $ " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " $ " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );

        }   
        else if (Settings::getInstance()->get_currency_name() == "EUR") 
        {
            double price = Settings::getInstance()->getEURPrice();
            double volume = Settings::getInstance()->getEURVolume();
            double cap =  Settings::getInstance()->getEURCAP();
            main->statusLabel->setText(
                "HUSH/EUR "+(QLocale(QLocale::German).toString(price,'f', 2))+ " €"
                );
            ui->volumeExchange->setText(
                QLocale(QLocale::German).toString(volume,'f', 2)+ " €"
            );
            ui->marketcapTab->setText(
                QLocale(QLocale::German).toString(cap,'f', 2)+ " €"
            );

        }
        else if (Settings::getInstance()->get_currency_name() == "BTC") 
        {
            double price = Settings::getInstance()->getBTCPrice();
            double volume = Settings::getInstance()->getBTCVolume();
            double cap =  Settings::getInstance()->getBTCCAP();
            main->statusLabel->setText(
                " HUSH/BTC=BTC " + (QLocale(QLocale::English).toString(price, 'f',8))
            );
            ui->volumeExchange->setText(
                " BTC " + (QLocale(QLocale::English).toString(volume, 'f',8))
            );
            ui->marketcapTab->setText(
                " BTC " + (QLocale(QLocale::English).toString(cap, 'f',8))
            );

        }
        else if (Settings::getInstance()->get_currency_name() == "CNY")
        {
            double price = Settings::getInstance()->getCNYPrice();
            double volume = Settings::getInstance()->getCNYVolume();
            double cap =  Settings::getInstance()->getCNYCAP();
            main->statusLabel->setText(
                " HUSH/CNY=¥ /元 " + (QLocale(QLocale::Chinese).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " ¥ /元  " + (QLocale(QLocale::Chinese).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " ¥ /元  " + (QLocale(QLocale::Chinese).toString(cap,'f', 2))
            );

        }   
        else if (Settings::getInstance()->get_currency_name() == "RUB") 
        {
            double price = Settings::getInstance()->getRUBPrice();
            double volume = Settings::getInstance()->getRUBVolume();
            double cap =  Settings::getInstance()->getRUBCAP();
            main->statusLabel->setText(
                " HUSH/RUB=₽ " + (QLocale(QLocale::German).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " ₽  " + (QLocale(QLocale::German).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " ₽  " + (QLocale(QLocale::German).toString(cap,'f', 2))
            );

        } 
        else if (Settings::getInstance()->get_currency_name() == "CAD") 
        {
            double price = Settings::getInstance()->getCADPrice();
            double volume = Settings::getInstance()->getCADVolume();
            double cap =  Settings::getInstance()->getCADCAP();
            main->statusLabel->setText(
                " HUSH/CAD=$ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " $ " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " $ " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );

        }
        else if  (Settings::getInstance()->get_currency_name() == "SGD") 
        {
            double price = Settings::getInstance()->getSGDPrice();
            double volume = Settings::getInstance()->getSGDVolume();
            double cap =  Settings::getInstance()->getSGDCAP();
            main->statusLabel->setText(
                " HUSH/SGD=$ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " $ " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " $ " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );

        }
        else if  (Settings::getInstance()->get_currency_name() == "CHF") 
        {
            double price = Settings::getInstance()->getCHFPrice();
            double volume = Settings::getInstance()->getCHFVolume();
            double cap =  Settings::getInstance()->getCHFCAP();
            main->statusLabel->setText(
                " HUSH/CHF= " + (QLocale(QLocale::German).toString(price,'f', 2))+ " CHF"
            );
            ui->volumeExchange->setText(
                QLocale(QLocale::German).toString(volume,'f', 2)+ " CHF"
            );
            ui->marketcapTab->setText(
                QLocale(QLocale::German).toString(cap,'f', 2)+ " CHF"
            );

        }
        else if (Settings::getInstance()->get_currency_name() == "INR") 
        {
            double price = Settings::getInstance()->getINRPrice();
            double volume = Settings::getInstance()->getINRVolume();
            double cap =  Settings::getInstance()->getINRCAP();
            main->statusLabel->setText(
                " HUSH/INR=₹ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " ₹  " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " ₹  " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );

        }
        else if  (Settings::getInstance()->get_currency_name() == "GBP") 
        {
            double price = Settings::getInstance()->getGBPPrice();
            double volume = Settings::getInstance()->getGBPVolume();
            double cap =  Settings::getInstance()->getGBPCAP();
            main->statusLabel->setText(
                " HUSH/GBP=£ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " £  " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " £  " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );

        }
        else if  (Settings::getInstance()->get_currency_name() == "AUD") 
        {
            double price = Settings::getInstance()->getAUDPrice();
            double volume = Settings::getInstance()->getAUDVolume();
            double cap =  Settings::getInstance()->getAUDCAP();
            main->statusLabel->setText(
                " HUSH/AUD=$ " + (QLocale(QLocale::English).toString(price,'f', 2))
            );
            ui->volumeExchange->setText(
                " $ " + (QLocale(QLocale::English).toString(volume,'f', 2))
            );
            ui->marketcapTab->setText(
                " $ " + (QLocale(QLocale::English).toString(cap,'f', 2))
            );
            
        } 
        else 
        {
            main->statusLabel->setText(
                " HUSH/USD=$" + QString::number(Settings::getInstance()->getZECPrice(),'f',2 )
            );
            ui->volumeExchange->setText(
                " $  " + QString::number((double)  Settings::getInstance()->getUSDVolume() ,'f',2)
            );
            ui->marketcapTab->setText(
                " $  " + QString::number((double)  Settings::getInstance()->getUSDCAP() ,'f',2)
            );
        }
        main->statusLabel->setToolTip(tooltip);
        main->statusIcon->setPixmap(i.pixmap(16, 16));
        main->statusIcon->setToolTip(tooltip);
        // See if recurring payments needs anything
        Recurring::getInstance()->processPending(main);

        // Check if the wallet is locked/encrypted
        zrpc->fetchWalletEncryptionStatus([=] (const json& reply) {
            bool isEncrypted = reply["encrypted"].get<json::boolean_t>();
            bool isLocked = reply["locked"].get<json::boolean_t>();
            model->setEncryptionStatus(isEncrypted, isLocked);
        });
          // Get the total supply and render it with thousand decimal
        zrpc->fetchSupply([=] (const json& reply) {   
            int supply  = reply["supply"].get<json::number_integer_t>();
            int zfunds = reply["zfunds"].get<json::number_integer_t>();
            int total = reply["total"].get<json::number_integer_t>();;
            if (
                Settings::getInstance()->get_currency_name() == "EUR" || 
                Settings::getInstance()->get_currency_name() == "CHF" || 
                Settings::getInstance()->get_currency_name() == "RUB"
            ) 
            {
                ui->supply_taddr->setText((QLocale(QLocale::German).toString(supply)+ " Hush"));
                ui->supply_zaddr->setText((QLocale(QLocale::German).toString(zfunds)+ " Hush"));
                ui->supply_total->setText((QLocale(QLocale::German).toString(total)+ " Hush"));
            }
            else
            {
                ui->supply_taddr->setText("Hush " + (QLocale(QLocale::English).toString(supply)));
                ui->supply_zaddr->setText("Hush " +(QLocale(QLocale::English).toString(zfunds)));
                ui->supply_total->setText("Hush " +(QLocale(QLocale::English).toString(total)));
            }

        });

        if ( doUpdate ) 
        {
            // Something changed, so refresh everything.
            refreshBalances();        
            refreshAddresses();     // This calls refreshZSentTransactions() and refreshReceivedZTrans()
            refreshTransactions();
        }
         refreshBalances();
        int lag = longestchain - notarized ;
        this->setLag(lag);
    }, [=](QString err) {
        // hushd has probably disappeared.
        
        this->noConnection();

        // Prevent multiple dialog boxes, because these are called async
        static bool shown = false;
        if (!shown && prevCallSucceeded) // show error only first time
        { 
            shown = true;
            QMessageBox::critical(
                main, 
                QObject::tr("Connection Error"), 
                QObject::tr("There was an error connecting to the server. Please check your internet connection. The error was") + ": \n\n"+ err,
                QMessageBox::StandardButton::Ok
            );
            shown = false;
        }

        prevCallSucceeded = false;
    });
}

int Controller::getLag()
{

    return _lag;

}

void Controller::setLag(int lag)
{

    _lag = lag;

}

void Controller::refreshAddresses() 
{
    if (!zrpc->haveConnection()) 
        return noConnection();
    
    auto newzaddresses = new QList<QString>();
    auto newtaddresses = new QList<QString>();
    zrpc->fetchAddresses([=] (json reply) {
        auto zaddrs = reply["z_addresses"].get<json::array_t>();
        for (auto& it : zaddrs) 
        {   
            auto addr = QString::fromStdString(it.get<json::string_t>());
            newzaddresses->push_back(addr);
        }

        model->replaceZaddresses(newzaddresses);
        auto taddrs = reply["t_addresses"].get<json::array_t>();
        for (auto& it : taddrs)
        {   
            auto addr = QString::fromStdString(it.get<json::string_t>());
            if (Settings::isTAddress(addr))
                newtaddresses->push_back(addr);
        }

        model->replaceTaddresses(newtaddresses);
        // Refresh the sent and received txs from all these z-addresses
        refreshTransactions();
    });
    
}

// Function to create the data model and update the views, used below.
void Controller::updateUI(bool anyUnconfirmed) 
{    
    ui->unconfirmedWarning->setVisible(anyUnconfirmed);
    // Update balances model data, which will update the table too
    balancesTableModel->setNewData(
        model->getAllZAddresses(), 
        model->getAllTAddresses(), 
        model->getAllBalances(), 
        model->getUTXOs()
    );
};

// Function to process reply of the listunspent and z_listunspent API calls, used below.
void Controller::processUnspent(const json& reply, QMap<QString, CAmount>* balancesMap, QList<UnspentOutput>* unspentOutputs) {
    auto processFn = [=](const json& array) {        
        for (auto& it : array) 
        {
            QString qsAddr  = QString::fromStdString(it["address"]);
            int block       = it["created_in_block"].get<json::number_unsigned_t>();
            QString txid    = QString::fromStdString(it["created_in_txid"]);
            CAmount amount  = CAmount::fromqint64(it["value"].get<json::number_unsigned_t>());

            bool spendable = it["unconfirmed_spent"].is_null() && it["spent"].is_null();    // TODO: Wait for 1 confirmations
            bool pending   = !it["unconfirmed_spent"].is_null();;

            unspentOutputs->push_back(
                UnspentOutput{ qsAddr, txid, amount, block, spendable, pending }
            );
            if (spendable) 
            {
                (*balancesMap)[qsAddr] = (*balancesMap)[qsAddr] +
                                         CAmount::fromqint64(it["value"].get<json::number_unsigned_t>());
            }
        }
    };

    processFn(reply["unspent_notes"].get<json::array_t>());
    processFn(reply["utxos"].get<json::array_t>());
    processFn(reply["pending_notes"].get<json::array_t>());
    processFn(reply["pending_utxos"].get<json::array_t>());
};

void Controller::updateUIBalances() 
{
    CAmount balT = getModel()->getBalT();
    CAmount balZ = getModel()->getBalZ();
    CAmount balVerified = getModel()->getBalVerified();

    // Reduce the BalanceZ by the pending outgoing amount. We're adding
    // here because totalPending is already negative for outgoing txns.
    balZ = balZ + getModel()->getTotalPending();

    CAmount balTotal     = balT + balZ;
    CAmount balAvailable = balT + balVerified;
    if (balZ < 0) 
        balZ = CAmount::fromqint64(0);
            double price = (Settings::getInstance()->getBTCPrice() / 1000);
      //  ui->PriceMemo->setText(" The price of \n one HushChat \n Message is :\n BTC " + (QLocale(QLocale::English).toString(price, 'f',8))
        //+ " Messages left :" + ((balTotal.toDecimalhushString()) /0.0001)  );
    // Balances table
    ui->balSheilded->setText(balZ.toDecimalhushString());
    ui->balVerified->setText(balVerified.toDecimalhushString());
    ui->balTransparent->setText(balT.toDecimalhushString());
    ui->balTotal->setText(balTotal.toDecimalhushString());

    if (Settings::getInstance()->get_currency_name() == "USD") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalUSDString());
        ui->balVerified->setToolTip(balVerified.toDecimalUSDString());
        ui->balTransparent->setToolTip(balT.toDecimalUSDString());
        ui->balTotal->setToolTip(balTotal.toDecimalUSDString());

    } 
    else if (Settings::getInstance()->get_currency_name() == "EUR") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalEURString());
        ui->balVerified->setToolTip(balVerified.toDecimalEURString());
        ui->balTransparent->setToolTip(balT.toDecimalEURString());
        ui->balTotal->setToolTip(balTotal.toDecimalEURString());

    }
    else if (Settings::getInstance()->get_currency_name() == "BTC") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalBTCString());
        ui->balVerified->setToolTip(balVerified.toDecimalBTCString());
        ui->balTransparent->setToolTip(balT.toDecimalBTCString());
        ui->balTotal->setToolTip(balTotal.toDecimalBTCString()); 
    
    } 
    else if (Settings::getInstance()->get_currency_name() == "CNY") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalCNYString());
        ui->balVerified->setToolTip(balVerified.toDecimalCNYString());
        ui->balTransparent->setToolTip(balT.toDecimalCNYString());
        ui->balTotal->setToolTip(balTotal.toDecimalCNYString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "RUB") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalRUBString());
        ui->balVerified->setToolTip(balVerified.toDecimalRUBString());
        ui->balTransparent->setToolTip(balT.toDecimalRUBString());
        ui->balTotal->setToolTip(balTotal.toDecimalRUBString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "CAD") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalCADString());
        ui->balVerified->setToolTip(balVerified.toDecimalCADString());
        ui->balTransparent->setToolTip(balT.toDecimalCADString());
        ui->balTotal->setToolTip(balTotal.toDecimalCADString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "SGD") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalSGDString());
        ui->balVerified->setToolTip(balVerified.toDecimalSGDString());
        ui->balTransparent->setToolTip(balT.toDecimalSGDString());
        ui->balTotal->setToolTip(balTotal.toDecimalSGDString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "CHF") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalCHFString());
        ui->balVerified->setToolTip(balVerified.toDecimalCHFString());
        ui->balTransparent->setToolTip(balT.toDecimalCHFString());
        ui->balTotal->setToolTip(balTotal.toDecimalCHFString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "INR") 
    {
        ui->balSheilded->setToolTip(balZ.toDecimalINRString());
        ui->balVerified->setToolTip(balVerified.toDecimalINRString());
        ui->balTransparent->setToolTip(balT.toDecimalINRString());
        ui->balTotal->setToolTip(balTotal.toDecimalINRString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "GBP") 
    {
        ui->balSheilded   ->setToolTip(balZ.toDecimalGBPString());
        ui->balVerified   ->setToolTip(balVerified.toDecimalGBPString());
        ui->balTransparent->setToolTip(balT.toDecimalGBPString());
        ui->balTotal      ->setToolTip(balTotal.toDecimalGBPString()); 

    } 
    else if (Settings::getInstance()->get_currency_name() == "AUD") 
    {
        ui->balSheilded   ->setToolTip(balZ.toDecimalAUDString());
        ui->balVerified   ->setToolTip(balVerified.toDecimalAUDString());
        ui->balTransparent->setToolTip(balT.toDecimalAUDString());
        ui->balTotal      ->setToolTip(balTotal.toDecimalAUDString()); 
    }
    // Send tab
    ui->txtAvailablehush->setText(balAvailable.toDecimalhushString());

    if (Settings::getInstance()->get_currency_name() == "USD")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalUSDString());

    else if (Settings::getInstance()->get_currency_name() == "EUR")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalEURString()); 

    else if (Settings::getInstance()->get_currency_name() == "BTC")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalBTCString()); 

    else if (Settings::getInstance()->get_currency_name() == "CNY")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalCNYString()); 

    else if (Settings::getInstance()->get_currency_name() == "RUB")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalRUBString()); 

    else if (Settings::getInstance()->get_currency_name() == "CAD")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalCADString()); 

    else if (Settings::getInstance()->get_currency_name() == "SGD")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalSGDString());

    else if (Settings::getInstance()->get_currency_name() == "CHF")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalCHFString()); 

    else if (Settings::getInstance()->get_currency_name() == "INR")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalINRString());

    else if (Settings::getInstance()->get_currency_name() == "GBP")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalGBPString());

    else if (Settings::getInstance()->get_currency_name() == "AUD")
        ui->txtAvailableUSD->setText(balAvailable.toDecimalAUDString()); 
}

void Controller::refreshBalances() 
{    
    if (!zrpc->haveConnection()) 
        return noConnection();

    // 1. Get the Balances
    zrpc->fetchBalance([=] (json reply) {    
        CAmount balT        = CAmount::fromqint64(reply["tbalance"].get<json::number_unsigned_t>());
        CAmount balZ        = CAmount::fromqint64(reply["zbalance"].get<json::number_unsigned_t>());
        CAmount balVerified = CAmount::fromqint64(reply["verified_zbalance"].get<json::number_unsigned_t>());
        
        model->setBalT(balT);
        model->setBalZ(balZ);
        model->setBalVerified(balVerified);

        // This is for the websockets
        AppDataModel::getInstance()->setBalances(balT, balZ);
        
        // This is for the datamodel
        CAmount balAvailable = balT + balVerified;
        model->setAvailableBalance(balAvailable);
        updateUIBalances();
    });

    // 2. Get the UTXOs
    // First, create a new UTXO list. It will be replacing the existing list when everything is processed.
    auto newUnspentOutputs = new QList<UnspentOutput>();
    auto newBalances = new QMap<QString, CAmount>();

    // Call the Transparent and Z unspent APIs serially and then, once they're done, update the UI
    zrpc->fetchUnspent([=] (json reply) {
        processUnspent(reply, newBalances, newUnspentOutputs);

        // Swap out the balances and UTXOs
        model->replaceBalances(newBalances);
        model->replaceUTXOs(newUnspentOutputs);

        // Find if any output is not spendable or is pending
        bool anyUnconfirmed = std::find_if(
            newUnspentOutputs->constBegin(), 
            newUnspentOutputs->constEnd(), 
            [=](const UnspentOutput& u) -> bool { 
                return !u.spendable ||  u.pending; 
            }
        ) != newUnspentOutputs->constEnd();

        updateUI(anyUnconfirmed);

        main->balancesReady();
    });
}

void Controller::refreshTransactions() {   
    if (!zrpc->haveConnection()) 
        return noConnection();

    zrpc->fetchTransactions([=] (json reply) {
        QList<TransactionItem> txdata;        

        for (auto& it : reply.get<json::array_t>()) {
        {
            QString address;
            CAmount total_amount;
            QList<TransactionItemDetail> items;

            long confirmations;
            if (it.find("unconfirmed") != it.end() && it["unconfirmed"].get<json::boolean_t>()) {
                confirmations = 0;
            }else{
                confirmations = model->getLatestBlock() - it["block_height"].get<json::number_integer_t>() + 1;
            }

            
            auto txid = QString::fromStdString(it["txid"]);
            auto datetime = it["datetime"].get<json::number_integer_t>();

            // First, check if there's outgoing metadata
            if (!it["outgoing_metadata"].is_null()) {
            
                for (auto o: it["outgoing_metadata"].get<json::array_t>())
                 
                {    
                    // if (chatModel->getCidByTx(txid) == QString("0xdeadbeef")){
                    QString address;
    
                    address = QString::fromStdString(o["address"]);

                    // Sent items are -ve
                    CAmount amount = CAmount::fromqint64(-1* o["value"].get<json::number_unsigned_t>());
                    
                    // Check for Memos
                   
                    if (confirmations == 0) {
                        chatModel->addconfirmations(txid, confirmations);
                    }
                    
                    if ((confirmations > 0)  && (chatModel->getConfirmationByTx(txid) != QString("0xdeadbeef"))) {
                        DataStore::getChatDataStore()->clear();
                        chatModel->killConfirmationCache();
                        chatModel->killMemoCache();
                        this->refresh(true);
                    }

                    QString memo;
                    QString cid;
                    QString headerbytes;
                    QString publickey;
                    if (!o["memo"].is_null()) {
                    memo = QString::fromStdString(o["memo"].get<json::string_t>());
                    
                        if (memo.startsWith("{")) {
                            try
                            {
                                QJsonDocument headermemo = QJsonDocument::fromJson(memo.toUtf8());

                                cid = headermemo["cid"].toString();
                                headerbytes = headermemo["e"].toString();

                                chatModel->addCid(txid, cid);
                                chatModel->addHeader(txid, headerbytes);

                            }
                            catch (...)
                            {
                                // on any exception caught
                            }
                        }
                          
                        bool isNotarized;

                        if (confirmations > getLag())
                        {
                            isNotarized = true;
                        }
                        else
                        {
                            isNotarized = false;
                        } 

                        if (chatModel->getCidByTx(txid) != QString("0xdeadbeef"))
                        {
                            cid = chatModel->getCidByTx(txid);
                        }
                        else
                        {
                            cid = "";
                        }


                        if (chatModel->getHeaderByTx(txid) != QString("0xdeadbeef"))
                        {
                            headerbytes = chatModel->getHeaderByTx(txid);
                        }
                        else
                        {
                            headerbytes = "";
                        }

                        if (main->getPubkeyByAddress(address) != QString("0xdeadbeef"))
                        {
                            publickey = main->getPubkeyByAddress(address);
                        }
                        else
                        {
                            publickey = "";
                        }
                    
                        /////We need to filter out Memos smaller then the ciphertext size, or it will dump
                        if ((memo.startsWith("{") == false) && (headerbytes.length() > 20))
                        {
                            QString passphrase = DataStore::getChatDataStore()->getPassword();
                            int length = passphrase.length();

                            ////////////////Generate the secretkey for our message encryption
                            char *hashEncryptionKeyraw = NULL;
                            hashEncryptionKeyraw = new char[length+1];
                            strncpy(hashEncryptionKeyraw, passphrase.toUtf8(), length +1);

                            const QByteArray pubkeyBobArray = QByteArray::fromHex(publickey.toLatin1());
                            const unsigned char *pubkeyBob = reinterpret_cast<const unsigned char *>(pubkeyBobArray.constData());
         
                            #define MESSAGEAS1 ((const unsigned char *) hashEncryptionKeyraw) ///////////
                            #define MESSAGEAS1_LEN length

                            unsigned char sk[crypto_kx_SECRETKEYBYTES];
                            unsigned char pk[crypto_kx_PUBLICKEYBYTES];
      
                            if (crypto_kx_seed_keypair(pk, sk, MESSAGEAS1) !=0)
                            {
                                main->logger->write("Keypair outgoing error");
                            }
                
                            unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];

      
                            ////////////////Get the pubkey from Bob, so we can create the share key

                            /////Create the shared key for sending the message

                            if (crypto_kx_server_session_keys(server_rx, server_tx, pk, sk, pubkeyBob) != 0)
                            {
                                 main->logger->write("Suspicious client public outgoing key, bail out ");
                            }
                
                            const QByteArray ba = QByteArray::fromHex(memo.toUtf8());
                            const unsigned char *encryptedMemo = reinterpret_cast<const unsigned char *>(ba.constData());

                            const QByteArray ba1 = QByteArray::fromHex(headerbytes.toLatin1());
                            const unsigned char *header = reinterpret_cast<const unsigned char *>(ba1.constData());

                            int encryptedMemoSize1 = ba.length();

                            QString memodecrypt;

                            if ((encryptedMemoSize1 - crypto_secretstream_xchacha20poly1305_ABYTES) > 0)
                            {
                                //////unsigned char* as message from QString
                                #define MESSAGE2 (const unsigned char *) encryptedMemo

                                ///////// length of the encrypted message
                                #define CIPHERTEXT1_LEN  encryptedMemoSize1

                                ///////Message length is smaller then the encrypted message
                                #define MESSAGE1_LEN encryptedMemoSize1 - crypto_secretstream_xchacha20poly1305_ABYTES

                                //////Set the length of the decrypted message

                                unsigned char decrypted[MESSAGE1_LEN];
                                unsigned char tag[crypto_secretstream_xchacha20poly1305_TAG_FINAL];
                                crypto_secretstream_xchacha20poly1305_state state;

                                /////Our decrypted message is now in decrypted. We need it as QString to render it
                                /////Only the QString gives weird data, so convert first to std::string
                                //   crypto_secretstream_xchacha20poly1305_keygen(client_rx);
                                if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, server_tx) != 0) {
                                    /* Invalid header, no need to go any further */
                                }
 
                                if (crypto_secretstream_xchacha20poly1305_pull
                                    (&state, decrypted, NULL, tag, MESSAGE2, CIPHERTEXT1_LEN, NULL, 0) != 0) {
                                    /* Invalid/incomplete/corrupted ciphertext - abort */
                                }

                                std::string decryptedMemo(reinterpret_cast<char*>(decrypted),MESSAGE1_LEN);
            
                                memodecrypt = QString::fromUtf8( decryptedMemo.data(), decryptedMemo.size());
                            }
                            else
                            {
                               
                                memodecrypt = "";
                            }

                            /////Now we can convert it to QString
                            //////////////Give us the output of the decrypted message as debug to see if it was successfully

                            ChatItem item = ChatItem(
                                datetime,
                                address,
                                QString(""),
                                memodecrypt,
                                QString(""),
                                QString(""),
                                cid, 
                                txid,
                                confirmations,
                                true,
                                isNotarized,
                                false
                            );

                            DataStore::getChatDataStore()->setData(ChatIDGenerator::getInstance()->generateID(item), item);
                           // updateUIBalances();
                        }
                    }

                    items.push_back(TransactionItemDetail{address, amount, memo});
                    total_amount = total_amount + amount;

                }
                
                {
                    QList<QString> addresses;
                    for (auto item : items) {
                        // Concat all the addresses
                        addresses.push_back(item.address);
                        address = addresses.join(",");
                    }
                }
                        
                txdata.push_back(TransactionItem{
                   "send", datetime, address, txid,confirmations, items
                });
                
            }
            else
            {
               
               { // Incoming Transaction
                address = (it["address"].is_null() ? "" : QString::fromStdString(it["address"]));
                model->markAddressUsed(address);

                QString memo;
                if (!it["memo"].is_null()) {
                    memo = QString::fromStdString(it["memo"]);

                items.push_back(TransactionItemDetail{
                        address,
                    CAmount::fromqint64(it["amount"].get<json::number_integer_t>()),
                        memo
                });

                TransactionItem tx{
                    "Receive", datetime, address, txid,confirmations, items
                };

                txdata.push_back(tx);
               }

           
                    QString type;
                    QString publickey;
                    QString headerbytes;
                    QString cid;
                    QString requestZaddr;
                    QString contactname;
                    bool isContact;

                if (!it["memo"].is_null()) {

                if (memo.startsWith("{")) {
                 try 
                 {
                  QJsonDocument headermemo = QJsonDocument::fromJson(memo.toUtf8());

                  cid = headermemo["cid"].toString();
                  type = headermemo["t"].toString();
                  requestZaddr =  headermemo["z"].toString();
                  headerbytes = headermemo["e"].toString();
                  publickey = headermemo["p"].toString();

                    chatModel->addCid(txid, cid);
                    chatModel->addrequestZaddr(txid, requestZaddr);
                    chatModel->addHeader(txid, headerbytes);

                if (publickey.length() > 10){
                    main->addPubkey(requestZaddr, publickey);
                }

                        }
                        catch (...)
                        {
                            // on any exception
                        }
                    }
  
                    if (chatModel->getCidByTx(txid) != QString("0xdeadbeef"))
                    {
                        cid = chatModel->getCidByTx(txid);
                    }
                    else
                    {
                        cid = "";
                    }
    
                    if (chatModel->getrequestZaddrByTx(txid) != QString("0xdeadbeef"))
                    {
                        requestZaddr = chatModel->getrequestZaddrByTx(txid);
                    }
                    else
                    {
                        requestZaddr = "";
                    }

                    if (chatModel->getHeaderByTx(txid) != QString("0xdeadbeef"))
                    {
                        headerbytes = chatModel->getHeaderByTx(txid);
                    }
                    else
                    {
                        headerbytes = "";
                    }

                    if (main->getPubkeyByAddress(requestZaddr) != QString("0xdeadbeef"))
                    {
                        publickey = main->getPubkeyByAddress(requestZaddr);
                    }
                    else
                    {
                        publickey = "";
                    }

                    if (contactModel->getContactbyAddress(requestZaddr) != QString("0xdeadbeef"))
                    {
                         isContact = true;
                         contactname = contactModel->getContactbyAddress(requestZaddr);
                    }
                    else
                    {
                         isContact = false;
                         contactname = "";
                    }

                    bool isNotarized;

                    if (confirmations > getLag())
                    {
                        isNotarized = true;
                    }
                    else
                    {
                        isNotarized = false;
                    }

                    int position = it["position"].get<json::number_integer_t>(); 

                    int ciphercheck = memo.length() - crypto_secretstream_xchacha20poly1305_ABYTES;

                    if ((memo.startsWith("{") == false) && (headerbytes > 0) && (ciphercheck > 0))
                    {
                        if (chatModel->getMemoByTx(txid) == QString("0xdeadbeef"))
                        {
                            if (position == 1)
                            {
                                chatModel->addMemo(txid, headerbytes);
                            }
                            else
                            {
                                //
                            }

                            QString passphrase = DataStore::getChatDataStore()->getPassword();
                            int length = passphrase.length();

                            char *hashEncryptionKeyraw = NULL;
                            hashEncryptionKeyraw = new char[length+1];
                            strncpy(hashEncryptionKeyraw, passphrase.toUtf8(), length +1);

                            const QByteArray pubkeyBobArray = QByteArray::fromHex(publickey.toLatin1());
                            const unsigned char *pubkeyBob = reinterpret_cast<const unsigned char *>(pubkeyBobArray.constData());

                            #define MESSAGEAS1 ((const unsigned char *) hashEncryptionKeyraw)///////////
                            #define MESSAGEAS1_LEN length

                            unsigned char sk[crypto_kx_SECRETKEYBYTES];
                            unsigned char pk[crypto_kx_PUBLICKEYBYTES];

                            if (crypto_kx_seed_keypair(pk, sk, MESSAGEAS1) !=0)
                            {
                               main->logger->write("Suspicious  outgoing key pair, bail out ");
                            }

                            unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];

                            ////////////////Get the pubkey from Bob, so we can create the share key

                            /////Create the shared key for sending the message

                            if (crypto_kx_client_session_keys(client_rx, client_tx, pk, sk, pubkeyBob) != 0)
                            {
                               main->logger->write("Suspicious client public incoming key, bail out ");
                            }

                            const QByteArray ba = QByteArray::fromHex(memo.toUtf8());
                            const unsigned char *encryptedMemo = reinterpret_cast<const unsigned char *>(ba.constData());

                            const QByteArray ba1 = QByteArray::fromHex(headerbytes.toLatin1());
                            const unsigned char *header = reinterpret_cast<const unsigned char *>(ba1.constData());

                            int encryptedMemoSize1 = ba.length();
                            int headersize = ba1.length();

                            //////unsigned char* as message from QString
                            #define MESSAGE2 (const unsigned char *) encryptedMemo

                            ///////// length of the encrypted message
                            #define CIPHERTEXT1_LEN  encryptedMemoSize1

                            ///////Message length is smaller then the encrypted message
                            #define MESSAGE1_LEN encryptedMemoSize1 - crypto_secretstream_xchacha20poly1305_ABYTES

                            //////Set the length of the decrypted message

                            unsigned char decrypted[MESSAGE1_LEN+1];
                            unsigned char tag[crypto_secretstream_xchacha20poly1305_TAG_FINAL];
                            crypto_secretstream_xchacha20poly1305_state state;

                            /////Our decrypted message is now in decrypted. We need it as QString to render it
                            /////Only the QString gives weird data, so convert first to std::string
                            //   crypto_secretstream_xchacha20poly1305_keygen(client_rx);
                            if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, client_rx) != 0) {
                               main->logger->write("Invalid header incoming, no need to go any further "); 
                            }

                            if (crypto_secretstream_xchacha20poly1305_pull
                                (&state, decrypted, NULL, tag, MESSAGE2, CIPHERTEXT1_LEN, NULL, 0) != 0) {
                                 main->logger->write("Invalid/incomplete/corrupted ciphertext - abort");
                            }

                            std::string decryptedMemo(reinterpret_cast<char*>(decrypted),MESSAGE1_LEN);

                            /////Now we can convert it to QString
                            QString memodecrypt;

                            memodecrypt = QString::fromUtf8( decryptedMemo.data(), decryptedMemo.size());

                            // }
                            //////////////Give us the output of the decrypted message as debug to see if it was successfully

                            ChatItem item = ChatItem(
                                datetime,
                                address,
                                contactname,
                                memodecrypt,
                                requestZaddr,
                                type,
                                cid,
                                txid,
                                confirmations,
                                false,
                                isNotarized,
                                isContact
                            );

                            DataStore::getChatDataStore()->setData(ChatIDGenerator::getInstance()->generateID(item), item);

                        }
                        else
                        {
                            //
                        }

                    }
                    else
                    {
                        ChatItem item = ChatItem(
                            datetime,
                            address,
                            contactname,
                            memo,
                            requestZaddr,
                            type,
                            cid,
                            txid,
                            confirmations,
                            false,
                            isNotarized,
                            isContact
                        );
                        DataStore::getChatDataStore()->setData(ChatIDGenerator::getInstance()->generateID(item), item);
                    }
                }
               }
            }
            }
        }

        // Calculate the total unspent amount that's pending. This will need to be
        // shown in the UI so the user can keep track of pending funds
        CAmount totalPending;
        for (auto txitem : txdata) {
            if (txitem.confirmations == 0) {
                for (auto item: txitem.items) {
                    totalPending = totalPending + item.amount;
                }
            }
        }

        getModel()->setTotalPending(totalPending);

        // Update UI Balance
        updateUIBalances();

        // Update model data, which updates the table view
        transactionsTableModel->replaceData(txdata);
        chat->renderChatBox(ui, ui->listChat,ui->memoSizeChat);
        ui->listChat->verticalScrollBar()->setValue(
        ui->listChat->verticalScrollBar()->maximum());
        
    });
    
}

void Controller::refreshChat(QListView *listWidget, QLabel *label)
{
    chat->renderChatBox(ui, listWidget, label);
    ui->listChat->verticalScrollBar()->setValue(
    ui->listChat->verticalScrollBar()->maximum());
  
}

void Controller::refreshContacts(QListView *listWidget)
{
    contactModel->renderContactList(listWidget);
    ui->listChat->verticalScrollBar()->setValue(
        ui->listChat->verticalScrollBar()->maximum());
}

// If the wallet is encrpyted and locked, we need to unlock it 
void Controller::unlockIfEncrypted(std::function<void(void)> cb, std::function<void(void)> error) 
{
    auto encStatus = getModel()->getEncryptionStatus();
    if (encStatus.first && encStatus.second) 
    {
        // Wallet is encrypted and locked. Ask for the password and unlock.
        QString password = QInputDialog::getText(
            main, 
            main->tr("Wallet Password"), 
            main->tr("Your wallet is encrypted.\nPlease enter your wallet password"), 
            QLineEdit::Password
        );

        if (password.isEmpty()) 
        {
            QMessageBox::critical(
                main, 
                main->tr("Wallet Decryption Failed"),
                main->tr("Please enter a valid password"),
                QMessageBox::Ok
            );
            error();
            return;
        }

        zrpc->unlockWallet(password, [=](json reply) {
            if (isJsonResultSuccess(reply)) 
            {
                cb();

                // Refresh the wallet so the encryption status is now in sync.
                refresh(true);
            } 
            else 
            {
                QMessageBox::critical(
                    main, 
                    main->tr("Wallet Decryption Failed"),
                    QString::fromStdString(reply["error"].get<json::string_t>()),
                    QMessageBox::Ok
                );
                error();
            }
        });

    } 
    else 
    {
        // Not locked, so just call the function
        cb();
    }
}

/**
 * Execute a transaction with the standard UI. i.e., standard status bar message and standard error
 * handling
 */
void Controller::executeStandardUITransaction(Tx tx) 
{
    executeTransaction(tx, [=] (QString txid) { 
            ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
        },
        [=] (QString opid, QString errStr) {
            ui->statusBar->showMessage(
                QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000
            );

            if (!opid.isEmpty())
                errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr; 

            QMessageBox::critical(
                main, 
                QObject::tr("Transaction Error"), 
                errStr, 
                QMessageBox::Ok
            );            
        }
    );
}


// Execute a transaction!
void Controller::executeTransaction(Tx tx, 
        const std::function<void(QString txid)> submitted,
        const std::function<void(QString txid, QString errStr)> error) 
{
    unlockIfEncrypted([=] () {
        // First, create the json params
        json params = json::array();
        fillTxJsonParams(params, tx);
        std::cout << std::setw(2) << params << std::endl;

        zrpc->sendTransaction(QString::fromStdString(params.dump()), [=](const json& reply) {
            if (reply.find("txid") == reply.end())
            {
                error("", "Couldn't understand Response: " + QString::fromStdString(reply.dump()));
            }
            else
            {
                QString txid = QString::fromStdString(reply["txid"].get<json::string_t>());
                submitted(txid);
            }
        },
        [=](QString errStr) {
            error("", errStr);
        });
    }, [=]() {
        error("", main->tr("Failed to unlock wallet"));
    });
}


void Controller::checkForUpdate(bool silent) 
{
    if (!zrpc->haveConnection()) 
        return noConnection();

    QUrl cmcURL("https://api.github.com/repos/MyHush/SilentDragonLite/releases");

    QNetworkRequest req;
    req.setUrl(cmcURL);
    
    QNetworkAccessManager *manager = new QNetworkAccessManager(this->main);
    QNetworkReply *reply = manager->get(req);

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        manager->deleteLater();

        try 
        {
            if (reply->error() == QNetworkReply::NoError) 
            {
                auto releases = QJsonDocument::fromJson(reply->readAll()).array();
                QVersionNumber maxVersion(0, 0, 0);
                for (QJsonValue rel : releases) 
                {
                    if (!rel.toObject().contains("tag_name"))
                        continue;

                    QString tag = rel.toObject()["tag_name"].toString();
                    if (tag.startsWith("v"))
                        tag = tag.right(tag.length() - 1);

                    if (!tag.isEmpty()) 
                    {
                        auto v = QVersionNumber::fromString(tag);
                        if (v > maxVersion)
                            maxVersion = v;
                    }
                }

                auto currentVersion = QVersionNumber::fromString(APP_VERSION);
                
                // Get the max version that the user has hidden updates for
                QSettings s;
                auto maxHiddenVersion = QVersionNumber::fromString(
                        s.value("update/lastversion", "0.0.0"
                    ).toString());

                qDebug() << "Version check: Current " << currentVersion << ", Available " << maxVersion;

                if (maxVersion > currentVersion && (!silent || maxVersion > maxHiddenVersion)) 
                {
                    auto ans = QMessageBox::information(main, QObject::tr("Update Available"), 
                        QObject::tr("A new release v%1 is available! You have v%2.\n\nWould you like to visit the releases page?")
                            .arg(maxVersion.toString())
                            .arg(currentVersion.toString()),
                        QMessageBox::Yes, QMessageBox::Cancel);
                    if (ans == QMessageBox::Yes) 
                    {
                        QDesktopServices::openUrl(QUrl("https://github.com/MyHush/SilentDragonLite/releases"));
                    } 
                    else 
                    {
                        // If the user selects cancel, don't bother them again for this version
                        s.setValue("update/lastversion", maxVersion.toString());
                    }

                } 
                else 
                {
                    if (!silent) 
                    {
                        QMessageBox::information(main, QObject::tr("No updates available"), 
                            QObject::tr("You already have the latest release v%1")
                                .arg(currentVersion.toString()));
                    }
                } 
            }
        }
        catch (...) 
        {
            // If anything at all goes wrong, just set the price to 0 and move on.
            qDebug() << QString("Caught something nasty");
        }       
    });
}

// Get the hush->USD price from coinmarketcap using their API
void Controller::refreshZECPrice() 
{
    if (!zrpc->haveConnection()) 
        return noConnection();

       // TODO: use/render all this data
    QUrl cmcURL("https://api.coingecko.com/api/v3/simple/price?ids=hush&vs_currencies=btc%2Cusd%2Ceur%2Ceth%2Cgbp%2Ccny%2Cjpy%2Crub%2Ccad%2Csgd%2Cchf%2Cinr%2Caud%2Cinr&include_market_cap=true&include_24hr_vol=true&include_24hr_change=true");
   
    QNetworkRequest req;
    req.setUrl(cmcURL);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this->main);
    QNetworkReply *reply = manager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, [=] {
        reply->deleteLater();
        manager->deleteLater();

        try 
        {
            if (reply->error() != QNetworkReply::NoError) 
            {
                auto parsed = json::parse(reply->readAll(), nullptr, false);
                if (!parsed.is_discarded() && !parsed["error"]["message"].is_null()) 
                    qDebug() << QString::fromStdString(parsed["error"]["message"]);
                else
                    qDebug() << reply->errorString();

                Settings::getInstance()->setZECPrice(0);
                Settings::getInstance()->setEURPrice(0);
                Settings::getInstance()->setBTCPrice(0);
                Settings::getInstance()->setCNYPrice(0);
                Settings::getInstance()->setRUBPrice(0);
                Settings::getInstance()->setCADPrice(0);
                Settings::getInstance()->setSGDPrice(0);
                Settings::getInstance()->setCHFPrice(0);
                Settings::getInstance()->setGBPPrice(0);
                Settings::getInstance()->setAUDPrice(0);
                Settings::getInstance()->setINRPrice(0);
                Settings::getInstance()->setUSDVolume(0);
                Settings::getInstance()->setEURVolume(0);
                Settings::getInstance()->setBTCVolume(0);
                Settings::getInstance()->setCNYVolume(0);
                Settings::getInstance()->setRUBVolume(0);
                Settings::getInstance()->setCADVolume(0);
                Settings::getInstance()->setINRVolume(0);
                Settings::getInstance()->setSGDVolume(0);
                Settings::getInstance()->setCHFVolume(0);
                Settings::getInstance()->setGBPVolume(0);
                Settings::getInstance()->setAUDVolume(0);
                Settings::getInstance()->setUSDCAP(0);
                Settings::getInstance()->setEURCAP(0);
                Settings::getInstance()->setBTCCAP(0);
                Settings::getInstance()->setCNYCAP(0);
                Settings::getInstance()->setRUBCAP(0);
                Settings::getInstance()->setCADCAP(0);
                Settings::getInstance()->setINRCAP(0);
                Settings::getInstance()->setSGDCAP(0);
                Settings::getInstance()->setCHFCAP(0);
                Settings::getInstance()->setGBPCAP(0);
                Settings::getInstance()->setAUDCAP(0);
                return;
            }

            qDebug() << "No network errors";
            auto all = reply->readAll();
            auto parsed = json::parse(all, nullptr, false);
            if (parsed.is_discarded()) 
            {
                Settings::getInstance()->setZECPrice(0);
                Settings::getInstance()->setEURPrice(0);
                Settings::getInstance()->setBTCPrice(0);
                Settings::getInstance()->setCNYPrice(0);
                Settings::getInstance()->setRUBPrice(0);
                Settings::getInstance()->setCADPrice(0);
                Settings::getInstance()->setSGDPrice(0);
                Settings::getInstance()->setCHFPrice(0);
                Settings::getInstance()->setGBPPrice(0);
                Settings::getInstance()->setAUDPrice(0);
                Settings::getInstance()->setINRPrice(0);
                Settings::getInstance()->setUSDVolume(0);
                Settings::getInstance()->setEURVolume(0);
                Settings::getInstance()->setBTCVolume(0);
                Settings::getInstance()->setCNYVolume(0);
                Settings::getInstance()->setRUBVolume(0);
                Settings::getInstance()->setCADVolume(0);
                Settings::getInstance()->setINRVolume(0);
                Settings::getInstance()->setSGDVolume(0);
                Settings::getInstance()->setCHFVolume(0);
                Settings::getInstance()->setGBPVolume(0);
                Settings::getInstance()->setAUDVolume(0);
                Settings::getInstance()->setUSDCAP(0);
                Settings::getInstance()->setEURCAP(0);
                Settings::getInstance()->setBTCCAP(0);
                Settings::getInstance()->setCNYCAP(0);
                Settings::getInstance()->setRUBCAP(0);
                Settings::getInstance()->setCADCAP(0);
                Settings::getInstance()->setINRCAP(0);
                Settings::getInstance()->setSGDCAP(0);
                Settings::getInstance()->setCHFCAP(0);
                Settings::getInstance()->setGBPCAP(0);
                Settings::getInstance()->setAUDCAP(0);
                return;
            }
            qDebug() << "Parsed JSON";
            const json& item  = parsed.get<json::object_t>();
            const json& hush  = item["hush"].get<json::object_t>();

            if (hush["usd"] >= 0)
            {
                qDebug() << "Found hush key in price json";
                qDebug() << "HUSH = $" << QString::number((double)hush["usd"]);
                Settings::getInstance()->setZECPrice( hush["usd"] );
            }

            if (hush["eur"] >= 0)
            {
                qDebug() << "HUSH = €" << QString::number((double)hush["eur"]);
                Settings::getInstance()->setEURPrice(hush["eur"]);
            }

            if (hush["btc"] >= 0)
            {
                qDebug() << "HUSH = BTC" << QString::number((double)hush["btc"]);
                Settings::getInstance()->setBTCPrice( hush["btc"]);
            }

            if (hush["cny"] >= 0)
            {
                qDebug() << "HUSH = CNY" << QString::number((double)hush["cny"]);
                Settings::getInstance()->setCNYPrice( hush["cny"]);
            }

            if (hush["rub"] >= 0)
            {
                qDebug() << "HUSH = RUB" << QString::number((double)hush["rub"]);
                Settings::getInstance()->setRUBPrice( hush["rub"]);
            }

            if (hush["cad"] >= 0)
            {
                qDebug() << "HUSH = CAD" << QString::number((double)hush["cad"]);
                Settings::getInstance()->setCADPrice( hush["cad"]);
            }

            if (hush["sgd"] >= 0)
            {
                qDebug() << "HUSH = SGD" << QString::number((double)hush["sgd"]);
                Settings::getInstance()->setSGDPrice( hush["sgd"]);
            }

            if (hush["chf"] >= 0)
            {
                qDebug() << "HUSH = CHF" << QString::number((double)hush["chf"]);
                Settings::getInstance()->setCHFPrice( hush["chf"]);
            }

            if (hush["inr"] >= 0)
            {  
                qDebug() << "HUSH = INR" << QString::number((double)hush["inr"]);
                Settings::getInstance()->setINRPrice( hush["inr"]);
            }

            if (hush["gbp"] >= 0)
            {
                qDebug() << "HUSH = GBP" << QString::number((double)hush["gbp"]);
                Settings::getInstance()->setGBPPrice( hush["gbp"]);
            }

            if (hush["aud"] >= 0)
            {
                qDebug() << "HUSH = AUD" << QString::number((double)hush["aud"]);
                Settings::getInstance()->setAUDPrice( hush["aud"]);
            }

            if (hush["btc_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = usd_24h_vol" << QString::number((double)hush["usd_24h_vol"]);
                Settings::getInstance()->setUSDVolume( hush["usd_24h_vol"]);
            }

            if (hush["btc_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = euro_24h_vol" << QString::number((double)hush["eur_24h_vol"]);
                Settings::getInstance()->setEURVolume( hush["eur_24h_vol"]);
            }

            if (hush["btc_24h_vol"] >= 0)
            {              
                qDebug() << "HUSH = btc_24h_vol" << QString::number((double)hush["btc_24h_vol"]);
                Settings::getInstance()->setBTCVolume( hush["btc_24h_vol"]);
            }
            
           if (hush["cny_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = cny_24h_vol" << QString::number((double)hush["cny_24h_vol"]);
                Settings::getInstance()->setCNYVolume( hush["cny_24h_vol"]);
            }

            if (hush["rub_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = rub_24h_vol" << QString::number((double)hush["rub_24h_vol"]);
                Settings::getInstance()->setRUBVolume( hush["rub_24h_vol"]);
            }
            
            if (hush["cad_24h_vol"] >= 0)
            {
                qDebug() << "HUSH = cad_24h_vol" << QString::number((double)hush["cad_24h_vol"]);
                Settings::getInstance()->setCADVolume( hush["cad_24h_vol"]);
            }

            if (hush["sgd_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = sgd_24h_vol" << QString::number((double)hush["sgd_24h_vol"]);
                Settings::getInstance()->setSGDVolume( hush["sgd_24h_vol"]);
            }

            if (hush["chf_24h_vol"] >= 0)
            {              
                qDebug() << "HUSH = chf_24h_vol" << QString::number((double)hush["chf_24h_vol"]);
                Settings::getInstance()->setCHFVolume( hush["chf_24h_vol"]);
            }

            if (hush["inr_24h_vol"] >= 0)
            {              
                qDebug() << "HUSH = inr_24h_vol" << QString::number((double)hush["inr_24h_vol"]);
                Settings::getInstance()->setINRVolume( hush["inr_24h_vol"]);
            }

            if (hush["gbp_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = gbp_24h_vol" << QString::number((double)hush["gbp_24h_vol"]);
                Settings::getInstance()->setGBPVolume( hush["gbp_24h_vol"]);
            }

            if (hush["aud_24h_vol"] >= 0)
            {  
                qDebug() << "HUSH = aud_24h_vol" << QString::number((double)hush["aud_24h_vol"]);
                Settings::getInstance()->setAUDVolume( hush["aud_24h_vol"]);
            }

            if (hush["usd_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = usd_market_cap" << QString::number((double)hush["usd_market_cap"]);
                Settings::getInstance()->setUSDCAP( hush["usd_market_cap"]);
            }

            if (hush["eur_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = eur_market_cap" << QString::number((double)hush["eur_market_cap"]);
                Settings::getInstance()->setEURCAP( hush["eur_market_cap"]);
            }

            if (hush["btc_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = btc_market_cap" << QString::number((double)hush["btc_market_cap"]);
                Settings::getInstance()->setBTCCAP( hush["btc_market_cap"]);
            }

            if (hush["cny_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = cny_market_cap" << QString::number((double)hush["cny_market_cap"]);
                Settings::getInstance()->setCNYCAP( hush["cny_market_cap"]);
            }

            if (hush["rub_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = rub_market_cap" << QString::number((double)hush["rub_market_cap"]);
                Settings::getInstance()->setRUBCAP( hush["rub_market_cap"]);
            }

            if (hush["cad_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = cad_market_cap" << QString::number((double)hush["cad_market_cap"]);
                Settings::getInstance()->setCADCAP( hush["cad_market_cap"]);
            }

            if (hush["sgd_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = sgd_market_cap" << QString::number((double)hush["sgd_market_cap"]);
                Settings::getInstance()->setSGDCAP( hush["sgd_market_cap"]);
            }

            if (hush["chf_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = chf_market_cap" << QString::number((double)hush["chf_market_cap"]);
                Settings::getInstance()->setCHFCAP( hush["chf_market_cap"]);
            }

            if (hush["inr_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = inr_market_cap" << QString::number((double)hush["inr_market_cap"]);
                Settings::getInstance()->setINRCAP( hush["inr_market_cap"]);
            }

            if (hush["gbp_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = gbp_market_cap" << QString::number((double)hush["gbp_market_cap"]);
                Settings::getInstance()->setGBPCAP( hush["gbp_market_cap"]);
            }

            if (hush["aud_market_cap"] >= 0)
            {  
                qDebug() << "HUSH = aud_market_cap" << QString::number((double)hush["aud_market_cap"]);
                Settings::getInstance()->setAUDCAP( hush["aud_market_cap"]);
            }

            return;
        } 
        catch (const std::exception& e) 
        {
            // If anything at all goes wrong, just set the price to 0 and move on.
            qDebug() << QString("Caught something nasty: ") << e.what();
        }

        // If nothing, then set the price to 0;
        Settings::getInstance()->setZECPrice(0);
        Settings::getInstance()->setEURPrice(0);
        Settings::getInstance()->setBTCPrice(0);
        Settings::getInstance()->setCNYPrice(0);
        Settings::getInstance()->setRUBPrice(0);
        Settings::getInstance()->setCADPrice(0);
        Settings::getInstance()->setSGDPrice(0);
        Settings::getInstance()->setCHFPrice(0);
        Settings::getInstance()->setGBPPrice(0);
        Settings::getInstance()->setAUDPrice(0);
        Settings::getInstance()->setINRPrice(0);
        Settings::getInstance()->setBTCVolume(0);
        Settings::getInstance()->setUSDVolume(0);
        Settings::getInstance()->setEURVolume(0);
        Settings::getInstance()->setBTCVolume(0);
        Settings::getInstance()->setCNYVolume(0);
        Settings::getInstance()->setRUBVolume(0);
        Settings::getInstance()->setCADVolume(0);
        Settings::getInstance()->setINRVolume(0);
        Settings::getInstance()->setSGDVolume(0);
        Settings::getInstance()->setCHFVolume(0);
        Settings::getInstance()->setGBPVolume(0);
        Settings::getInstance()->setAUDVolume(0);
        Settings::getInstance()->setUSDCAP(0);
        Settings::getInstance()->setEURCAP(0);
        Settings::getInstance()->setBTCCAP(0);
        Settings::getInstance()->setCNYCAP(0);
        Settings::getInstance()->setRUBCAP(0);
        Settings::getInstance()->setCADCAP(0);
        Settings::getInstance()->setINRCAP(0);
        Settings::getInstance()->setSGDCAP(0);
        Settings::getInstance()->setCHFCAP(0);
        Settings::getInstance()->setGBPCAP(0);
        Settings::getInstance()->setAUDCAP(0);
    });

}

void Controller::shutdownhushd() 
{
    // Save the wallet and exit the lightclient library cleanly.
    if (zrpc->haveConnection()) 
    {
        QDialog d(main);
        Ui_ConnectionDialog connD;
        connD.setupUi(&d);
        auto theme = Settings::getInstance()->get_theme_name();
        auto size  = QSize(512,512);

    if (theme == "Dark" || theme == "Midnight") {
        QMovie *movie2 = new QMovie(":/img/res/silentdragonlite-animated-startup-dark.gif");;
        movie2->setScaledSize(size);
        qDebug() << "Animation dark loaded";
        connD.topIcon->setMovie(movie2);
        movie2->start();
        connD.status->setText(QObject::tr("Please wait for SilentDragonLite to exit"));
        connD.statusDetail->setText(QObject::tr("Waiting for hushd to exit"));
    } else {
        QMovie *movie1 = new QMovie(":/img/res/silentdragonlite-animated-startup.gif");;
        movie1->setScaledSize(size);
        qDebug() << "Animation light loaded";
        connD.topIcon->setMovie(movie1);
        movie1->start();
        connD.status->setText(QObject::tr("Please wait for SilentDragonLite to exit"));
        connD.statusDetail->setText(QObject::tr("Waiting for hushd to exit"));
    }
      
        bool finished = false;
        zrpc->saveWallet([&] (json) {
            if (!finished)
                d.accept();
            finished = true;
        });

        if (!finished)
            d.exec();
    }
}

/** 
 * Get a Sapling address from the user's wallet
 */ 
QString Controller::getDefaultSaplingAddress() 
{
    for (QString addr: model->getAllZAddresses()) 
    {
        if (Settings::getInstance()->isSaplingAddress(addr))
            return addr;
    }

    return QString();
}

QString Controller::getDefaultTAddress() 
{
    if (model->getAllTAddresses().length() > 0)
        return model->getAllTAddresses().at(0);

    else 
        return QString();
        
}
