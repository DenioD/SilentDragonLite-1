// Copyright 2019-2020 The Hush developers
// GPLv3

#include "mainwindow.h"
#include "addressbook.h"
#include "viewalladdresses.h"
#include "ui_encryption.h"
#include "ui_mainwindow.h"
#include "ui_mobileappconnector.h"
#include "ui_addressbook.h"
#include "ui_privkey.h"
#include "ui_about.h"
#include "ui_settings.h"
#include "ui_viewalladdresses.h"
#include "controller.h"
#include "balancestablemodel.h"
#include "settings.h"
#include "version.h"
#include "connection.h"
#include "ui_sendHushTransactionChat.h"
#include "ui_contactrequest.h"
#include "ui_deposithush.h"
#include "ui_emoji.h"
#include "ui_requestContactDialog.h"
#include "chatmodel.h"
#include "requestdialog.h"
#include "ui_startupencryption.h" 
#include "ui_removeencryption.h"
#include "ui_seedrestore.h"
#include "websockets.h"
#include "sodium.h"
#include "sodium/crypto_generichash_blake2b.h"
#include <QRegularExpression>
#include "FileSystem/FileSystem.h"
#include "Crypto/passwd.h"
#include "Crypto/FileEncryption.h"
#include "DataStore/DataStore.h"
#include "firsttimewizard.h"
#include "../lib/silentdragonlitelib.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeyEvent>

using json = nlohmann::json;


#ifdef Q_OS_WIN
auto dirwallet = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletenc = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackup = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.datBackup");
#endif
#ifdef Q_OS_MACOS
auto dirwallet = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletenc = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackup = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.datBackup");
#endif
#ifdef Q_OS_LINUX
auto dirwallet = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletenc = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackup = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet.datBackup");
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   
	// Include css    
    QString theme_name;
    try
    {
       theme_name = Settings::getInstance()->get_theme_name();
    }
    catch (...)
    {
        theme_name = "Dark";
    }

    this->slot_change_theme(theme_name);

 
    ui->setupUi(this);

    logger = new Logger(this, QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite-wallet.log"));
      // Check for encryption
 
       
 
    if(fileExists(dirwalletenc))
    {
        this->removeWalletEncryptionStartUp();
    }

     ui->memoTxtChat->setAutoFillBackground(false);
     ui->memoTxtChat->setPlaceholderText("Send Message (you can only write messages after the initial message from your contact)");
     ui->memoTxtChat->setTextColor(Qt::white);
    
    // Status Bar
    setupStatusBar();
    
    // Settings editor 
    setupSettingsModal();

    // Set up exit action
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Set up Feedback action
    QObject::connect(ui->actionDonate, &QAction::triggered, this, &MainWindow::donate);

    QObject::connect(ui->actionDiscord, &QAction::triggered, this, &MainWindow::discord);

    QObject::connect(ui->actionWebsite, &QAction::triggered, this, &MainWindow::website);

    // File a bug
    QObject::connect(ui->actionFile_a_bug, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/MyHush/SilentDragonLite/issues/new"));
    });

    // Set up check for updates action
    QObject::connect(ui->actionCheck_for_Updates, &QAction::triggered, [=] () {
        // Silent is false, so show notification even if no update was found
        rpc->checkForUpdate(false);
    });

    // Recurring payments 
    QObject::connect(ui->action_Recurring_Payments, &QAction::triggered, [=]() {
        Recurring::getInstance()->showRecurringDialog(this);
    });

    // Request hush
    QObject::connect(ui->actionRequest_hush, &QAction::triggered, [=]() {
        RequestDialog::showRequesthush(this);
    });

    // Pay hush URI
    QObject::connect(ui->actionPay_URI, &QAction::triggered, [=] () {
        payhushURI();
    });

    // Wallet encryption
    QObject::connect(ui->actionEncrypt_Wallet, &QAction::triggered, [=]() {
        encryptWallet();
    });

    QObject::connect(ui->actionRemove_Wallet_Encryption, &QAction::triggered, [=]() {
        removeWalletEncryption();
    });

    // Export All Private Keys
    QObject::connect(ui->actionExport_All_Private_Keys, &QAction::triggered, this, &MainWindow::exportAllKeys);

    // Backup wallet.dat
    QObject::connect(ui->actionExport_Seed, &QAction::triggered, this, &MainWindow::exportSeed);

    // Export transactions
    QObject::connect(ui->actionExport_transactions, &QAction::triggered, this, &MainWindow::exportTransactions);

    // Connect mobile app
    QObject::connect(ui->actionConnect_Mobile_App, &QAction::triggered, this, [=] () {
        if (rpc->getConnection() == nullptr)
            return;

        AppDataServer::getInstance()->connectAppDialog(this);
    });

    // Rescan
    QObject::connect(ui->actionRescan, &QAction::triggered, [=]() {

       /* QFile file(dirwalletenc);
        QFile file1(dirwallet);

        if(fileExists(dirwalletenc))

          {
        file.remove();
        file1.remove();
          }*/


    Ui_Restore restoreSeed;
    QDialog dialog(this);
    restoreSeed.setupUi(&dialog);
    Settings::saveRestore(&dialog);


            rpc->fetchSeed([=](json reply) {
        if (isJsonError(reply)) {
            return;
        }

        restoreSeed.seed->setReadOnly(true);
        restoreSeed.seed->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
        QString seedJson = QString::fromStdString(reply["seed"].get<json::string_t>());
        restoreSeed.seed->setPlainText(seedJson);

        QString birthday = QString::number(reply["birthday"].get<json::number_unsigned_t>());
        restoreSeed.birthday->setPlainText(birthday);
        });

    QObject::connect(restoreSeed.restore, &QPushButton::clicked, [&](){

    QString seed = restoreSeed.seed->toPlainText();
    if (seed.trimmed().split(" ").length() != 24) {
        QMessageBox::warning(this, tr("Failed to restore wallet"), 
            tr("SilentDragonLite needs 24 words to restore wallet"),
            QMessageBox::Ok);
        return false;
    }


    // 2. Validate birthday
    QString birthday_str =  restoreSeed.birthday->toPlainText();
    bool ok;
    qint64 birthday = birthday_str.toUInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Failed to parse wallet birthday"), 
            tr("Couldn't understand wallet birthday. This should be a block height from where to rescan the wallet. You can leave it as '0' if you don't know what it should be."),
            QMessageBox::Ok);
        return false;
    }


    QString number_str =  restoreSeed.quantity->text();
    qint64 number = number_str.toUInt();

    auto config = std::shared_ptr<ConnectionConfig>(new ConnectionConfig());
    config->server = Settings::getInstance()->getSettings().server;
    // 3. Attempt to restore wallet with the seed phrase
    {
        char* resp = litelib_initialize_new_from_phrase(config->dangerous, config->server.toStdString().c_str(),
                seed.toStdString().c_str(), birthday, number);
        QString reply = litelib_process_response(resp);

        if (reply.toUpper().trimmed() != "OK") {
            QMessageBox::warning(this, tr("Failed to restore wallet"), 
                tr("Couldn't restore the wallet") + "\n" + reply,
                QMessageBox::Ok);
           
        } 
    }

    // 4. Finally attempt to save the wallet
    {
        char* resp = litelib_execute("save", "");
        QString reply = litelib_process_response(resp);

        QByteArray ba_reply = reply.toUtf8();
        QJsonDocument jd_reply = QJsonDocument::fromJson(ba_reply);
        QJsonObject parsed = jd_reply.object();

        if (parsed.isEmpty() || parsed["result"].isNull()) {
            QMessageBox::warning(this, tr("Failed to save wallet"), 
                tr("Couldn't save the wallet") + "\n" + reply,
                QMessageBox::Ok);

        } else {}  

            dialog.close();
          // To rescan, we clear the wallet state, and then reload the connection
        // This will start a sync, and show the scanning status. 
       this->getRPC()->clearWallet([=] (auto) {
            // Save the wallet
            this->getRPC()->saveWallet([=] (auto) {
                // Then reload the connection. The ConnectionLoader deletes itself.
               auto cl = new ConnectionLoader(this, rpc);
                cl->loadConnection();
                         });       
                     });

    
                 }

             });
               
        dialog.exec();
});

    // Address Book
    QObject::connect(ui->action_Address_Book, &QAction::triggered, this, &MainWindow::addressBook);

    // Set up about action
    QObject::connect(ui->actionAbout, &QAction::triggered, [=] () {
        QDialog aboutDialog(this);
        Ui_about about;
        about.setupUi(&aboutDialog);
        Settings::saveRestore(&aboutDialog);

        QString version    = QString("Version ") % QString(APP_VERSION) % " (" % QString(__DATE__) % ")";
        about.versionLabel->setText(version);
        
        aboutDialog.exec();
    });

    // Initialize to the balances tab
    ui->tabWidget->setCurrentIndex(0);


    // The hushd tab is hidden by default, and only later added in if the embedded hushd is started
    //hushdtab = ui->tabWidget->widget(4);
    //ui->tabWidget->removeTab(4);

    setupSendTab();
    setupTransactionsTab();
    setupReceiveTab();
    setupBalancesTab();
    setuphushdTab();
    setupchatTab();

    rpc = new Controller(this);

    restoreSavedStates();

    if (AppDataServer::getInstance()->isAppConnected()) {
        auto ads = AppDataServer::getInstance();

        QString wormholecode = "";
        if (ads->getAllowInternetConnection())
            wormholecode = ads->getWormholeCode(ads->getSecretHex());

        createWebsocket(wormholecode);
    }
}

bool MainWindow::fileExists(QString path) 
{
    QFileInfo check_file(path);
    return (check_file.exists() && check_file.isFile());
}
 
void MainWindow::createWebsocket(QString wormholecode) {
    qDebug() << "Listening for app connections on port 8777";
    // Create the websocket server, for listening to direct connections
    wsserver = new WSServer(8777, false, this);

    if (!wormholecode.isEmpty()) {
        // Connect to the wormhole service
        wormhole = new WormholeClient(this, wormholecode);
    }
}

void MainWindow::stopWebsocket() {
    delete wsserver;
    wsserver = nullptr;

    delete wormhole;
    wormhole = nullptr;

    qDebug() << "Websockets for app connections shut down";
}

bool MainWindow::isWebsocketListening() {
    return wsserver != nullptr;
}

void MainWindow::replaceWormholeClient(WormholeClient* newClient) {
    delete wormhole;
    wormhole = newClient;
}

void MainWindow::restoreSavedStates() {
    QSettings s;
    restoreGeometry(s.value("geometry").toByteArray());

    auto balance_geom = s.value("baltablegeom");
    if (balance_geom == QVariant()) {
        ui->balancesTable->setColumnWidth(0, 500);
    } else {
        ui->balancesTable->horizontalHeader()->restoreState(balance_geom.toByteArray());
    }

    auto tx_geom = s.value("tratablegeom");
    if (tx_geom == QVariant()) {
        ui->transactionsTable->setColumnWidth(1, 500);
    } else {
        ui->transactionsTable->horizontalHeader()->restoreState(tx_geom.toByteArray());
    }
}

void MainWindow::doClose() {
    closeEvent(nullptr);
}

void MainWindow::doClosePw() {
    closeEventpw(nullptr);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    QSettings s;

    s.setValue("geometry", saveGeometry());
    s.setValue("baltablegeom", ui->balancesTable->horizontalHeader()->saveState());
    s.setValue("tratablegeom", ui->transactionsTable->horizontalHeader()->saveState());

    s.sync();


    // Let the RPC know to shut down any running service.
    rpc->shutdownhushd();
    int passphraselenght = DataStore::getChatDataStore()->getPassword().length();

// Check is encryption is ON for SDl
    if(passphraselenght > 0) 
   
    {
        // delete old file before

        //auto dirHome =  QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        QFile fileoldencryption(dirwalletenc);
        fileoldencryption.remove();

         // Encrypt our wallet.dat 
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
        const unsigned char *encryptedMemo1 = reinterpret_cast<const unsigned char *>(ba.constData());
 
        auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QString sourceWallet_file = dirwallet;
        QString target_encWallet_file = dirwalletenc;
     
       // FileEncryption::encrypt(target_enc_file, source_file, key);
        FileEncryption::encrypt(target_encWallet_file, sourceWallet_file, encryptedMemo1);      

        QFile wallet(dirwallet);
        wallet.remove();
    }
    

    // Bubble up
    if (event)
        QMainWindow::closeEvent(event);
}

void MainWindow::closeEventpw(QCloseEvent* event) {

    // Let the RPC know to shut down any running service.
    rpc->shutdownhushd();


}


void MainWindow::encryptWallet() {

    QDialog d(this);
    Ui_encryptionDialog ed;
    ed.setupUi(&d);

    // Handle edits on the password box
    
    
    auto fnPasswordEdited = [=](const QString&) {
        // Enable the OK button if the passwords match.
        QString password = ed.txtPassword->text();
        
        if (!ed.txtPassword->text().isEmpty() && 
                ed.txtPassword->text() == ed.txtConfirmPassword->text() && password.size() >= 16) {
            ed.lblPasswordMatch->setText("");
            ed.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        } else {
            //ed.lblPasswordMatch->setText(tr("Passphrase don't match or You have entered too few letters (16 minimum)"));
            ed.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }

    };

    QObject::connect(ed.txtConfirmPassword, &QLineEdit::textChanged, fnPasswordEdited);
    QObject::connect(ed.txtPassword, &QLineEdit::textChanged, fnPasswordEdited);

    if (d.exec() == QDialog::Accepted) 
    {

    QString passphraseBlank = ed.txtPassword->text(); // data comes from user inputs
    QString passphrase = QString("HUSH3") + passphraseBlank + QString("SDL");
    int length = passphrase.length();
    

        char *sequence = NULL;
        sequence = new char[length+1];
        strncpy(sequence, passphrase.toUtf8(), length +1);
        
        QString passphraseHash = blake3_PW(sequence);
        DataStore::getChatDataStore()->setPassword(passphraseHash);

        char *sequence1 = NULL;
        sequence1 = new char[length+1];
        strncpy(sequence1, passphraseHash.toUtf8(), length+1);

        #define MESSAGE ((const unsigned char *) sequence)
        #define MESSAGE_LEN length
        #define hash ((const unsigned char *) sequence1)

        #define PASSWORD sequence
        #define KEY_LEN crypto_box_SEEDBYTES

        unsigned char key[KEY_LEN];

         if (crypto_pwhash
         (key, sizeof key, PASSWORD, strlen(PASSWORD), hash,
         crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
         crypto_pwhash_ALG_DEFAULT) != 0) {
         /* out of memory */
}
        QString passphraseHash1 = QByteArray(reinterpret_cast<const char*>(key), KEY_LEN).toHex();
        DataStore::getChatDataStore()->setPassword(passphraseHash1);

        auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        auto dirHome =  QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        QString sourceWallet_file = dirwallet;
        QString target_encWallet_file = dirwalletenc;
    
        FileEncryption::encrypt(target_encWallet_file, sourceWallet_file, key);

        QFile wallet(dirwallet);
        wallet.rename(dirwalletbackup);

           QMessageBox::information(this, tr("Wallet Encryption Success"),
                    QString("Successfully encrypted your wallet"),
                    QMessageBox::Ok
                ); 
    }
}

void MainWindow::removeWalletEncryption() {
    QDialog d(this);
    Ui_removeencryption ed;
    ed.setupUi(&d);

    if (fileExists(dirwalletenc) == false) {
        QMessageBox::information(this, tr("Wallet is not encrypted"), 
                    tr("Your wallet is not encrypted with a passphrase."),
                    QMessageBox::Ok
                );
        return;
    }

     auto fnPasswordEdited = [=](const QString&) {
        QString password = ed.txtPassword->text();
        // Enable the OK button if the passwords match.
        if (!ed.txtPassword->text().isEmpty() && 
                ed.txtPassword->text() == ed.txtConfirmPassword->text() && password.size() >= 16) {
            ed.lblPasswordMatch->setText("");
            ed.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        } else {
            ed.lblPasswordMatch->setText(tr("Passwords don't match"));
            ed.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }

    };

    QObject::connect(ed.txtConfirmPassword, &QLineEdit::textChanged, fnPasswordEdited);
    QObject::connect(ed.txtPassword, &QLineEdit::textChanged, fnPasswordEdited);

    if (d.exec() == QDialog::Accepted) 
    {
    QString passphraseBlank = ed.txtPassword->text(); // data comes from user inputs

    QString passphrase = QString("HUSH3") + passphraseBlank + QString("SDL");

    int length = passphrase.length();

    char *sequence = NULL;
    sequence = new char[length+1];
    strncpy(sequence, passphrase.toUtf8(), length +1);

    QString passphraseHash = blake3_PW(sequence);

    char *sequence1 = NULL;
    sequence1 = new char[length+1];
    strncpy(sequence1, passphraseHash.toUtf8(), length+1);


    #define hash ((const unsigned char *) sequence1)

    #define PASSWORD sequence
    #define KEY_LEN crypto_box_SEEDBYTES

    unsigned char key[KEY_LEN];

    if (crypto_pwhash
    (key, sizeof key, PASSWORD, strlen(PASSWORD), hash,
     crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
     crypto_pwhash_ALG_DEFAULT) != 0) {
    /* out of memory */
}
  
        auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        auto dirHome =  QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        QString target_encwallet_file = dirwalletenc;
        QString target_decwallet_file = dirwallet;

        FileEncryption::decrypt(target_decwallet_file, target_encwallet_file, key);

    
     QFile filencrypted(dirwalletenc);
     QFile wallet(dirwallet);
       
    if (wallet.size() > 0)
    {
      
         QMessageBox::information(this, tr("Wallet decryption Success"),
                    QString("Successfully delete the encryption"),
                    QMessageBox::Ok
                );   

        filencrypted.remove();  

        }else{
               
         QMessageBox::critical(this, tr("Wallet Encryption Failed"),
                    QString("False password, please try again"),
                    QMessageBox::Ok
                );
                 this->removeWalletEncryption();
        }    

    }
   
}

void MainWindow::removeWalletEncryptionStartUp() {
   QDialog d(this);
    Ui_startup ed;
    ed.setupUi(&d);
  
    if (d.exec() == QDialog::Accepted) 
    {
        QString passphraseBlank = ed.txtPassword->text(); // data comes from user inputs

        QString passphrase = QString("HUSH3") + passphraseBlank + QString("SDL");
        int length = passphrase.length();
        
        char *sequence = NULL;
        sequence = new char[length+1];
        strncpy(sequence, passphrase.toUtf8(), length +1);
        
        QString passphraseHash = blake3_PW(sequence);
        

        char *sequence1 = NULL;
        sequence1 = new char[length+1];
        strncpy(sequence1, passphraseHash.toUtf8(), length+1);

        #define MESSAGE ((const unsigned char *) sequence)
        #define MESSAGE_LEN length
        #define hash ((const unsigned char *) sequence1)

        #define PASSWORD sequence
        #define KEY_LEN crypto_box_SEEDBYTES

    unsigned char key[KEY_LEN];

    if (crypto_pwhash
    (key, sizeof key, PASSWORD, strlen(PASSWORD), hash,
     crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
     crypto_pwhash_ALG_DEFAULT) != 0) {
    /* out of memory */
}
        QString passphraseHash1 = QByteArray(reinterpret_cast<const char*>(key), KEY_LEN).toHex();
        DataStore::getChatDataStore()->setPassword(passphraseHash1);

        auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

        QString target_encwallet_file = dirwalletenc;
        QString target_decwallet_file = dirwallet;

        FileEncryption::decrypt(target_decwallet_file, target_encwallet_file, key);

    

     auto dirHome =  QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
     QFile wallet(dirwallet);
    
    if (wallet.size() == 0)
    {
         
         QMessageBox::critical(this, tr("Wallet Encryption Failed"),
                    QString("false password please try again"),
                    QMessageBox::Ok
                );
                 this->removeWalletEncryptionStartUp();
        }else{}   

    }else{

        this->doClosePw();
    }
  
}

QString MainWindow::getPassword()
{

    return _password;
}

void MainWindow::setPassword(QString password)
{

    _password = password;
}

QString MainWindow::getAmt()
{

    return _amt;
}

void MainWindow::setAmt(QString amt)
{

    _amt = amt;
}

QString MainWindow::getMoneyMemo()
{

    return _moneymemo;
}

void MainWindow::setMoneyMemo(QString moneymemo)
{

    _moneymemo = moneymemo;
}

void MainWindow::setupStatusBar() {
    // Status Bar
    loadingLabel = new QLabel();
    loadingMovie = new QMovie(":/icons/res/loading.gif");
    loadingMovie->setScaledSize(QSize(32, 16));
    loadingMovie->start();
    loadingLabel->setAttribute(Qt::WA_NoSystemBackground);
    loadingLabel->setMovie(loadingMovie);
    

    ui->statusBar->addPermanentWidget(loadingLabel);
    loadingLabel->setVisible(false);

    // Custom status bar menu
    ui->statusBar->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->statusBar, &QStatusBar::customContextMenuRequested, [=](QPoint pos) {
        auto msg = ui->statusBar->currentMessage();
        QMenu menu(this);

        if (!msg.isEmpty() && msg.startsWith(Settings::txidStatusMessage)) {
            auto txid = msg.split(":")[1].trimmed();
            menu.addAction(tr("Copy txid"), [=]() {
                QGuiApplication::clipboard()->setText(txid);
            });
            menu.addAction(tr("Copy block explorer link"), [=]() {
               // auto explorer = Settings::getInstance()->getExplorer();
             QGuiApplication::clipboard()->setText("https://explorer.myhush.org/tx/" + txid);
            });

            menu.addAction(tr("View tx on block explorer"), [=]() {
                Settings::openTxInExplorer(txid);
            });
        }

        menu.addAction(tr("Refresh"), [=]() {
            rpc->refresh(true);
        });
        QPoint gpos(mapToGlobal(pos).x(), mapToGlobal(pos).y() + this->height() - ui->statusBar->height());
        menu.exec(gpos);
    });

    statusLabel = new QLabel();
    ui->statusBar->addPermanentWidget(statusLabel);

    statusIcon = new QLabel();
    ui->statusBar->addPermanentWidget(statusIcon);
}

void MainWindow::setupSettingsModal() {    
    // Set up File -> Settings action
    QObject::connect(ui->actionSettings, &QAction::triggered, [=]() {
        QDialog settingsDialog(this);
        Ui_Settings settings;
        settings.setupUi(&settingsDialog);
        Settings::saveRestore(&settingsDialog);
    
     // Include currencies 

    QString currency_name;
    try
    {
       currency_name = Settings::getInstance()->get_currency_name();
    }
    catch (...)
    {
        currency_name = "USD";
    }

    this->slot_change_currency(currency_name);

        // Setup theme combo
        int theme_index = settings.comboBoxTheme->findText(Settings::getInstance()->get_theme_name(), Qt::MatchExactly);
        settings.comboBoxTheme->setCurrentIndex(theme_index);

        QObject::connect(settings.comboBoxTheme, &QComboBox::currentTextChanged, [=] (QString theme_name) {
            this->slot_change_theme(theme_name);
        });

        // Get Currency Data
       
        int currency_index = settings.comboBoxCurrency->findText(Settings::getInstance()->get_currency_name(), Qt::MatchExactly);
        settings.comboBoxCurrency->setCurrentIndex(currency_index);
        
       QObject::connect(settings.comboBoxCurrency, &QComboBox::currentTextChanged, [=] (QString currency_name) {
            this->slot_change_currency(currency_name);
            rpc->refresh(true);
            
             // Tell the user to restart
            QMessageBox::information(this, tr("Currency Change"), tr("This change can take a few seconds."), QMessageBox::Ok);  
             });

        // Check for updates
        settings.chkCheckUpdates->setChecked(Settings::getInstance()->getCheckForUpdates());

        // Fetch prices
        settings.chkFetchPrices->setChecked(Settings::getInstance()->getAllowFetchPrices());
        
        // List of default servers
        settings.cmbServer->addItem("https://lite.myhush.org");
        settings.cmbServer->addItem("6onaaujm4ozaokzu.onion:80");


        // Load current values into the dialog        
        auto conf = Settings::getInstance()->getSettings();
        settings.cmbServer->setCurrentText(conf.server);

        // Connection tab by default
        settings.tabWidget->setCurrentIndex(0);

        if (settingsDialog.exec() == QDialog::Accepted) {
            // Check for updates
            Settings::getInstance()->setCheckForUpdates(settings.chkCheckUpdates->isChecked());

            // Allow fetching prices
            Settings::getInstance()->setAllowFetchPrices(settings.chkFetchPrices->isChecked());

            // Save the server
            bool reloadConnection = false;
            if (conf.server != settings.cmbServer->currentText().trimmed()) {
                reloadConnection = true;
            }
            Settings::getInstance()->saveSettings(settings.cmbServer->currentText().trimmed());

            if (reloadConnection) {
                // Save settings
                Settings::getInstance()->saveSettings(settings.cmbServer->currentText());

                // Save the wallet
                getRPC()->saveWallet([=] (auto) {
                    // Then reload the connection. The ConnectionLoader deletes itself.
                    auto cl = new ConnectionLoader(this, rpc);
                    cl->loadConnection();
                });
            }
        }
    });
}

void MainWindow::addressBook() {
    // Check to see if there is a target.
    QRegularExpression re("Address[0-9]+", QRegularExpression::CaseInsensitiveOption);
    for (auto target: ui->sendToWidgets->findChildren<QLineEdit *>(re)) {
        if (target->hasFocus()) {
            AddressBook::open(this, target);
            return;
        }
    };

    // If there was no target, then just run with no target.
    AddressBook::open(this);
}

void MainWindow::discord() {
    QString url = "https://myhush.org/discord/";
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::website() {
    QString url = "https://myhush.org";
    QDesktopServices::openUrl(QUrl(url));
}


void MainWindow::donate() {
    // Set up a donation to me :)

    ui->Address1->setText(Settings::getDonationAddr());
    ui->Address1->setCursorPosition(0);
    ui->Amount1->setText("0.00");
    ui->MemoTxt1->setText(tr("Some feedback about SilentDragonlite or Hush..."));

    ui->statusBar->showMessage(tr("Send DenioD some private and shielded feedback about") % Settings::getTokenName() % tr(" or SilentDragonLite"));

    // And switch to the send tab.
    ui->tabWidget->setCurrentIndex(1);
}

// void MainWindow::doImport(QList<QString>* keys) {
//     if (rpc->getConnection() == nullptr) {
//         // No connection, just return
//         return;
//     }

//     if (keys->isEmpty()) {
//         delete keys;
//         ui->statusBar->showMessage(tr("Private key import rescan finished"));
//         return;
//     }

//     // Pop the first key
//     QString key = keys->first();
//     keys->pop_front();
//     bool rescan = keys->isEmpty();

//     if (key.startsWith("SK") ||
//         key.startsWith("secret")) { // Z key
//         rpc->importZPrivKey(key, rescan, [=] (auto) { this->doImport(keys); });                   
//     } else {
//         rpc->importTPrivKey(key, rescan, [=] (auto) { this->doImport(keys); });
//     }
// }


// Callback invoked when the RPC has finished loading all the balances, and the UI 
// is now ready to send transactions.
void MainWindow::balancesReady() {
    // First-time check
    if (uiPaymentsReady)
        return;

    uiPaymentsReady = true;
    qDebug() << "Payment UI now ready!";

    // There is a pending URI payment (from the command line, or from a secondary instance),
    // process it.
    if (!pendingURIPayment.isEmpty()) {
        qDebug() << "Paying hush URI";
        payhushURI(pendingURIPayment);
        pendingURIPayment = "";
    }

    // Execute any pending Recurring payments
    Recurring::getInstance()->processPending(this);
}

// Event filter for MacOS specific handling of payment URIs
bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent*>(event);
        if (!fileEvent->url().isEmpty())
            payhushURI(fileEvent->url().toString());

        return true;
    }

    return QObject::eventFilter(object, event);
}


// Pay the hush URI by showing a confirmation window. If the URI parameter is empty, the UI
// will prompt for one. If the myAddr is empty, then the default from address is used to send
// the transaction.
void MainWindow::payhushURI(QString uri, QString myAddr) {
    // If the Payments UI is not ready (i.e, all balances have not loaded), defer the payment URI
    if (!isPaymentsReady()) {
        qDebug() << "Payment UI not ready, waiting for UI to pay URI";
        pendingURIPayment = uri;
        return;
    }

    // If there was no URI passed, ask the user for one.
    if (uri.isEmpty()) {
        uri = QInputDialog::getText(this, tr("Paste HUSH URI"),
            "HUSH URI" + QString(" ").repeated(180));
    }

    // If there's no URI, just exit
    if (uri.isEmpty())
        return;

    // Extract the address
    qDebug() << "Received URI " << uri;
    PaymentURI paymentInfo = Settings::parseURI(uri);
    if (!paymentInfo.error.isEmpty()) {
        QMessageBox::critical(this, tr("Error paying HUSH URI"), 
                tr("URI should be of the form 'hush:<addr>?amt=x&memo=y") + "\n" + paymentInfo.error);
        return;
    }

    // Now, set the fields on the send tab
    clearSendForm();

    ui->Address1->setText(paymentInfo.addr);
    ui->Address1->setCursorPosition(0);
    ui->Amount1->setText(paymentInfo.amt);
    ui->MemoTxt1->setText(paymentInfo.memo);

    // And switch to the send tab.
    ui->tabWidget->setCurrentIndex(1);
    raise();

    // And click the send button if the amount is > 0, to validate everything. If everything is OK, it will show the confirm box
    // else, show the error message;
    if (paymentInfo.amt > 0) {
        sendButton();
    }
}


// void MainWindow::importPrivKey() {
//     QDialog d(this);
//     Ui_PrivKey pui;
//     pui.setupUi(&d);
//     Settings::saveRestore(&d);

//     pui.buttonBox->button(QDialogButtonBox::Save)->setVisible(false);
//     pui.helpLbl->setText(QString() %
//                         tr("Please paste your private keys (z-Addr or t-Addr) here, one per line") % ".\n" %
//                         tr("The keys will be imported into your connected hushd node"));  

//     if (d.exec() == QDialog::Accepted && !pui.privKeyTxt->toPlainText().trimmed().isEmpty()) {
//         auto rawkeys = pui.privKeyTxt->toPlainText().trimmed().split("\n");

//         QList<QString> keysTmp;
//         // Filter out all the empty keys.
//         std::copy_if(rawkeys.begin(), rawkeys.end(), std::back_inserter(keysTmp), [=] (auto key) {
//             return !key.startsWith("#") && !key.trimmed().isEmpty();
//         });

//         auto keys = new QList<QString>();
//         std::transform(keysTmp.begin(), keysTmp.end(), std::back_inserter(*keys), [=](auto key) {
//             return key.trimmed().split(" ")[0];
//         });

//         // Special case. 
//         // Sometimes, when importing from a paperwallet or such, the key is split by newlines, and might have 
//         // been pasted like that. So check to see if the whole thing is one big private key
//         if (Settings::getInstance()->isValidSaplingPrivateKey(keys->join(""))) {
//             auto multiline = keys;
//             keys = new QList<QString>();
//             keys->append(multiline->join(""));
//             delete multiline;
//         }

//         // Start the import. The function takes ownership of keys
//         QTimer::singleShot(1, [=]() {doImport(keys);});

//         // Show the dialog that keys will be imported. 
//         QMessageBox::information(this,
//             "Imported", tr("The keys were imported. It may take several minutes to rescan the blockchain. Until then, functionality may be limited"),
//             QMessageBox::Ok);
//     }
// }

/** 
 * Export transaction history into a CSV file
 */
void MainWindow::exportTransactions() {
    // First, get the export file name
    QString exportName = "hush-transactions-" + QDateTime::currentDateTime().toString("yyyyMMdd") + ".csv";

    QUrl csvName = QFileDialog::getSaveFileUrl(this, 
            tr("Export transactions"), exportName, "CSV file (*.csv)");

    if (csvName.isEmpty())
        return;

    if (!rpc->getTransactionsModel()->exportToCsv(csvName.toLocalFile())) {
        QMessageBox::critical(this, tr("Error"), 
            tr("Error exporting transactions, file was not saved"), QMessageBox::Ok);
    }
} 

/**
 * Export the seed phrase.
*/
void MainWindow::exportSeed() {
    if (!rpc->getConnection())
        return;

    

    rpc->fetchSeed([=](json reply) {
        if (isJsonError(reply)) {
            return;
        }
        
        QDialog d(this);
        Ui_PrivKey pui;
        pui.setupUi(&d);
        
        // Make the window big by default
        auto ps = this->geometry();
        QMargins margin = QMargins() + 50;
        d.setGeometry(ps.marginsRemoved(margin));

        Settings::saveRestore(&d);

        pui.privKeyTxt->setReadOnly(true);
        pui.privKeyTxt->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
        pui.privKeyTxt->setPlainText(QString::fromStdString(reply.dump()));
        
        pui.helpLbl->setText(tr("This is your wallet seed. Please back it up carefully and safely."));

        // Wire up save button
        QObject::connect(pui.buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=] () {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                            "Hush-seed.txt");
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
                return;
            }        
            QTextStream out(&file);
            out << pui.privKeyTxt->toPlainText();
        });

        pui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
        
        d.exec();
    });
}

void MainWindow::addPubkey(QString requestZaddr, QString pubkey)
{
    this->pubkeyMap[requestZaddr] = pubkey;
}

QString MainWindow::getPubkeyByAddress(QString requestZaddr)
{
    for(auto& pair : this->pubkeyMap)
    {

    }

    if(this->pubkeyMap.count(requestZaddr) > 0)
    {
        return this->pubkeyMap[requestZaddr];
    }

    return QString("0xdeadbeef");
}


void MainWindow::exportAllKeys() {
    exportKeys("");
}

void MainWindow::exportKeys(QString addr) {
    if (!rpc->getConnection())
        return;

    bool allKeys = addr.isEmpty() ? true : false;

    auto fnUpdateUIWithKeys = [=](json reply) {
        if (isJsonError(reply)) {
            return;                
        }

        if (reply.is_discarded() || !reply.is_array()) {
            QMessageBox::critical(this, tr("Error getting private keys"),
                tr("Error loading private keys: ") + QString::fromStdString(reply.dump()),
                QMessageBox::Ok);
            return;
        }
            
        QDialog d(this);
        Ui_PrivKey pui;
        pui.setupUi(&d);
        
        // Make the window big by default
        auto ps = this->geometry();
        QMargins margin = QMargins() + 50;
        d.setGeometry(ps.marginsRemoved(margin));

        Settings::saveRestore(&d);

        pui.privKeyTxt->setReadOnly(true);
        pui.privKeyTxt->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);

        if (allKeys)
            pui.helpLbl->setText(tr("These are all the private keys for all the addresses in your wallet"));
        else
            pui.helpLbl->setText(tr("Private key for ") + addr);


        // Wire up save button
        QObject::connect(pui.buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=] () {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                            allKeys ? "Hush-all-privatekeys.txt" : "Hush-privatekey.txt");
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
                return;
            }        
            QTextStream out(&file);
            out << pui.privKeyTxt->toPlainText();
        });

        QString allKeysTxt;
        for (auto i : reply.get<json::array_t>()) {
            allKeysTxt = allKeysTxt % QString::fromStdString(i["private_key"]) % " # addr=" % QString::fromStdString(i["address"]) % "\n";
        }

        pui.privKeyTxt->setPlainText(allKeysTxt);
        pui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);

        d.exec();
    };

    if (allKeys) {
        rpc->fetchAllPrivKeys(fnUpdateUIWithKeys);
    }
    else { 
        rpc->fetchPrivKey(addr, fnUpdateUIWithKeys);                
    }    
}

void MainWindow::setupBalancesTab() {
    ui->unconfirmedWarning->setVisible(false);
    ui->lblSyncWarning->setVisible(false);
    ui->lblSyncWarningReceive->setVisible(false);
    QObject::connect(ui->depositHushButton, &QPushButton::clicked, [=](){

    Ui_deposithush deposithush;
    QDialog dialog(this);
    deposithush.setupUi(&dialog);
    Settings::saveRestore(&dialog);

     QList<QString> allAddresses;

     allAddresses = getRPC()->getModel()->getAllZAddresses();
    QString depositzaddr = allAddresses[0];
     deposithush.qrcodeDisplayDeposit->setQrcodeString(depositzaddr);
     deposithush.zaddr->setText(depositzaddr);

      QObject::connect(deposithush.CopyAddress, &QPushButton::clicked, [=](){

        QGuiApplication::clipboard()->setText(depositzaddr);
        ui->statusBar->showMessage(tr("Copied to clipboard"), 3 * 1000);

      });



    dialog.exec();




    });


    // Setup context menu on balances tab
    ui->balancesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->balancesTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->balancesTable->indexAt(pos);
        if (index.row() < 0) return;

        index = index.sibling(index.row(), 0);
        auto addr = AddressBook::addressFromAddressLabel(
                            ui->balancesTable->model()->data(index).toString());

        QMenu menu(this);

        menu.addAction(tr("Copy address"), [=] () {
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(addr);            
            ui->statusBar->showMessage(tr("Copied to clipboard"), 3 * 1000);
        });

        menu.addAction(tr("Get private key"), [=] () {
            this->exportKeys(addr);
        });


        if (Settings::isTAddress(addr)) {
            menu.addAction(tr("View on block explorer"), [=] () {
                Settings::openAddressInExplorer(addr);
            });
        }

        menu.exec(ui->balancesTable->viewport()->mapToGlobal(pos));            
    });

}

void MainWindow::setuphushdTab() {    
    ui->hushdlogo->setBasePixmap(QPixmap(":/img/res/hushdlogo.gif"));
}

void MainWindow::setupTransactionsTab() {
    // Double click opens up memo if one exists
    QObject::connect(ui->transactionsTable, &QTableView::doubleClicked, [=] (auto index) {
        auto txModel = dynamic_cast<TxTableModel *>(ui->transactionsTable->model());
        QString memo = txModel->getMemo(index.row());

        if (!memo.isEmpty()) {
            QMessageBox mb(QMessageBox::Information, tr("Memo"), memo, QMessageBox::Ok, this);
            mb.setTextFormat(Qt::PlainText);
            mb.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
            mb.exec();
        }
    });

    // Set up context menu on transactions tab
    auto theme = Settings::getInstance()->get_theme_name();
    if (theme == "Dark" || theme == "Midnight") {
    ui->listChat->setStyleSheet("background-image: url(:/icons/res/SDLogo.png) ;background-attachment: fixed ;background-position: center center ;background-repeat: no-repeat");
     }
    if (theme == "Default") {ui->listChat->setStyleSheet("background-image: url(:/icons/res/sdlogo2.png) ;background-attachment: fixed ;background-position: center center ;background-repeat: no-repeat");}
   
    ui->listChat->setResizeMode(QListView::Adjust);
    ui->listChat->setWordWrap(true);
    ui->listChat->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listChat->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listChat->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listChat->setMinimumSize(200,350);
    ui->listChat->setItemDelegate(new ListViewDelegate());
    ui->listChat->show();
    
    ui->transactionsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    // Table right click
    QObject::connect(ui->transactionsTable, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
        QModelIndex index = ui->transactionsTable->indexAt(pos);
        if (index.row() < 0) return;

        QMenu menu(this);

        auto txModel = dynamic_cast<TxTableModel *>(ui->transactionsTable->model());

        QString txid = txModel->getTxId(index.row());
        QString memo = txModel->getMemo(index.row());
        QString addr = txModel->getAddr(index.row());

        menu.addAction(tr("Copy txid"), [=] () {            
            QGuiApplication::clipboard()->setText(txid);
            ui->statusBar->showMessage(tr("Copied to clipboard"), 3 * 1000);
        });

        if (!addr.isEmpty()) {
            menu.addAction(tr("Copy address"), [=] () {
                QGuiApplication::clipboard()->setText(addr);
                ui->statusBar->showMessage(tr("Copied to clipboard"), 3 * 1000);
            });
        }
           menu.addAction(tr("Copy block explorer link"), [=]() {
               // auto explorer = Settings::getInstance()->getExplorer();
             QGuiApplication::clipboard()->setText("https://explorer.myhush.org/tx/" + txid);
            });

        menu.addAction(tr("View on block explorer"), [=] () {
            Settings::openTxInExplorer(txid);
        });

        // Payment Request
        if (!memo.isEmpty() && memo.startsWith("hush:")) {
            menu.addAction(tr("View Payment Request"), [=] () {
                RequestDialog::showPaymentConfirmation(this, memo);
            });
        }

        // View Memo
        if (!memo.isEmpty()) {
            menu.addAction(tr("View Memo"), [=] () {               
                QMessageBox mb(QMessageBox::Information, tr("Memo"), memo, QMessageBox::Ok, this);
                mb.setTextFormat(Qt::PlainText);
                mb.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
                mb.exec();
            });
        }

        // If memo contains a reply to address, add a "Reply to" menu item
        if (!memo.isEmpty()) {
            int lastPost     = memo.trimmed().lastIndexOf(QRegExp("[\r\n]+"));
            QString lastWord = memo.right(memo.length() - lastPost - 1);
            
            if (Settings::getInstance()->isSaplingAddress(lastWord) || 
                Settings::getInstance()->isSproutAddress(lastWord)) {
                menu.addAction(tr("Reply to ") + lastWord.left(25) + "...", [=]() {
                    // First, cancel any pending stuff in the send tab by pretending to click
                    // the cancel button
                    cancelButton();

                    // Then set up the fields in the send tab
                    ui->Address1->setText(lastWord);
                    ui->Address1->setCursorPosition(0);
                    ui->Amount1->setText("0.0001");

                    // And switch to the send tab.
                    ui->tabWidget->setCurrentIndex(1);

                    qApp->processEvents();

                    // Click the memo button
                   this->memoButtonClicked(1, true);
                });
            }
        }

        menu.exec(ui->transactionsTable->viewport()->mapToGlobal(pos));        
    });
    
}

void MainWindow::setupchatTab() {

    ui->memoTxtChat->setEnabled(false);
    ui->emojiButton->setEnabled(false);
    ui->sendChatButton->setEnabled(false);

          /////////////Setting Icons for Chattab and different themes
       
  auto theme = Settings::getInstance()->get_theme_name();
        if (theme == "Dark" || theme == "Midnight") {
            QPixmap send(":/icons/res/send-white.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);

            QPixmap notification(":/icons/res/requestWhite.png");
            QIcon notificationIcon(notification);
            ui->pushContact->setIcon(notificationIcon);

            QPixmap addContact(":/icons/res/addContactWhite.png");
            QIcon addContactIcon(addContact);
            ui->safeContactRequest->setIcon(addContactIcon);

            QPixmap newAddr(":/icons/res/getAddrWhite.png");
            QIcon addnewAddrIcon(newAddr);
            ui->givemeZaddr->setIcon(addnewAddrIcon);

            ui->memoTxtChat->setTextColor("White");

        }else{
            
            QPixmap send(":/icons/res/sendBlack.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);

            QPixmap notification(":/icons/res/requestBlack.png");
            QIcon notificationIcon(notification);
            ui->pushContact->setIcon(notificationIcon);

            QPixmap addContact(":/icons/res/addContactBlack.png");
            QIcon addContactIcon(addContact);
            ui->safeContactRequest->setIcon(addContactIcon);

            QPixmap newAddr(":/icons/res/getAddrBlack.png");
            QIcon addnewAddrIcon(newAddr);
            ui->givemeZaddr->setIcon(addnewAddrIcon);

            ui->memoTxtChat->setTextColor("Black");
        }
    
    QObject::connect(ui->sendChatButton, &QPushButton::clicked, this, &MainWindow::sendChat);
    QObject::connect(ui->sendChatButton, &QPushButton::clicked, [&] () {

        ui->memoTxtChat->setEnabled(false);
        ui->emojiButton->setEnabled(false);
                

    });
    QObject::connect(ui->safeContactRequest, &QPushButton::clicked, this, &MainWindow::addContact);
    QObject::connect(ui->pushContact, &QPushButton::clicked, this , &MainWindow::renderContactRequest);

    ui->contactNameMemo->setText("");   

    /////Copy Chatmessages

     QMenu* contextMenuChat;
     QAction* copymessage;
     QAction* viewexplorer;
     QAction* copytxid;
     contextMenuChat = new QMenu(ui->listChat);
     copymessage = new QAction("Copy message to clipboard",contextMenuChat);
     viewexplorer = new QAction("View on block explorer",contextMenuChat);
     copytxid = new QAction("Copy txid to clipboard ",contextMenuChat);
    
 QObject::connect(ui->listContactWidget, &QTableView::clicked, [=] () {

     ui->listChat->setContextMenuPolicy(Qt::ActionsContextMenu);
     ui->listChat->addAction(copymessage);
     ui->listChat->addAction(viewexplorer);
     ui->listChat->addAction(copytxid);

 });

     QObject::connect(copymessage, &QAction::triggered, [=] {

    
    QModelIndex index = ui->listChat->currentIndex();
    QString memo_chat = index.data(Qt::DisplayRole).toString();
    QClipboard *clipboard = QGuiApplication::clipboard();
    int startPos = memo_chat.indexOf("<p>") + 3;
    int endPos = memo_chat.indexOf("</p>");
    int length = endPos - startPos;
    QString copymemo = memo_chat.mid(startPos, length);
   
    clipboard->setText(copymemo);
    ui->statusBar->showMessage(tr("Copied message to clipboard"), 3 * 1000);   

});
    QObject::connect(copytxid, &QAction::triggered, [=] {

    QModelIndex index = ui->listChat->currentIndex();
    QString memo_chat = index.data(Qt::DisplayRole).toString();
    QClipboard *clipboard = QGuiApplication::clipboard();

    int startPos = memo_chat.indexOf("<p>") + 3;
    int endPos = memo_chat.indexOf("</p>");
    int length = endPos - startPos;
    QString copymemo = memo_chat.mid(startPos, length);
    int startPosT = memo_chat.indexOf("<small>") + 7;
    int endPosT = memo_chat.indexOf("<b>");
    int lengthT = endPosT - startPosT;

    QString time = memo_chat.mid(startPosT, lengthT);

    for (auto &c : DataStore::getChatDataStore()->getAllRawChatItems()){

    if (c.second.getMemo() == copymemo)
    {
    int timestamp =  c.second.getTimestamp();
    QDateTime myDateTime;
    QString lock;
    myDateTime.setTime_t(timestamp);
    QString timestamphtml = myDateTime.toString("yyyy-MM-dd hh:mm");

    if(timestamphtml == time)

    {
    clipboard->setText(c.second.getTxid());
    ui->statusBar->showMessage(tr("Copied Txid to clipboard"), 3 * 1000);  
    }else{}

    }
   
}

});

    QObject::connect(viewexplorer, &QAction::triggered, [=] {

    QModelIndex index = ui->listChat->currentIndex();
    QString memo_chat = index.data(Qt::DisplayRole).toString();

    int startPos = memo_chat.indexOf("<p>") + 3;
    int endPos = memo_chat.indexOf("</p>");
    int length = endPos - startPos;
    QString copymemo = memo_chat.mid(startPos, length);
    int startPosT = memo_chat.indexOf("<small>") + 7;
    int endPosT = memo_chat.indexOf("<b>");
    int lengthT = endPosT - startPosT;

    QString time = memo_chat.mid(startPosT, lengthT);

    for (auto &c : DataStore::getChatDataStore()->getAllRawChatItems()){

    if (c.second.getMemo() == copymemo)
    {
    int timestamp =  c.second.getTimestamp();
    QDateTime myDateTime;
    QString lock;
    myDateTime.setTime_t(timestamp);
    QString timestamphtml = myDateTime.toString("yyyy-MM-dd hh:mm");

    if(timestamphtml == time)

    {

     Settings::openTxInExplorer(c.second.getTxid());
    
    }else{}

    }
   
}
});



///////// Add contextmenu 
     QMenu* contextMenu;
     QAction* requestAction;
     QAction* editAction;
     QAction* HushAction;
     QAction* requestHushAction;
     QAction* subatomicAction;
     contextMenu = new QMenu(ui->listContactWidget);
     HushAction = new QAction("Send or Request Hush ",contextMenu);
     editAction = new QAction("Delete this contact",contextMenu);
     subatomicAction = new QAction("Make a subatomic swap with a friend- coming soon",contextMenu);


///////// Set selected Zaddr for Chat with click

    QObject::connect(ui->listContactWidget, &QTableView::clicked, [=] () {

     ui->listContactWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
     ui->listContactWidget->addAction(HushAction);
     ui->listContactWidget->addAction(editAction); 
     ui->listContactWidget->addAction(subatomicAction);

     ui->memoTxtChat->setEnabled(false);
     ui->emojiButton->setEnabled(false);
     ui->sendChatButton->setEnabled(false);

        QModelIndex index = ui->listContactWidget->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();
        
        for(auto &p : AddressBook::getInstance()->getAllAddressLabels())
        if (label_contact == p.getName()) {
        ui->contactNameMemo->setText(p.getName());    
        rpc->refresh(true);
    
        }
   });

         QObject::connect(HushAction, &QAction::triggered, [=]() {   

        QModelIndex index = ui->listContactWidget->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();

        Ui_transactionHush transaction;
        QDialog transactionDialog(this);
        transaction.setupUi(&transactionDialog);
        Settings::saveRestore(&transactionDialog);
       // transaction.requestHush->setEnabled(false);
       // transaction.requestHush->setVisible(false);
        transaction.amountChat->setValidator(this->getAmountValidator());
        QString icon = ":icons/res/hush-money-white.png";
        QPixmap hush(icon);
        transaction.label_3->setPixmap(hush);

        

        for(auto &p : AddressBook::getInstance()->getAllAddressLabels())
        if (label_contact == p.getName()) {
            
        QStandardItemModel* contact = new QStandardItemModel();
        QString avatar = p.getAvatar();
        QStandardItem* Items1 = new QStandardItem(p.getName());
        Items1->setData(QIcon(avatar),Qt::DecorationRole);
        contact->appendRow(Items1); 
        transaction.contactName->setModel(contact);
        transaction.contactName->setIconSize(QSize(60,70));
        transaction.contactName->setUniformItemSizes(true);
        transaction.contactName->setDragDropMode(QAbstractItemView::DropOnly);      
        transaction.contactName->show();
        

        }
        
        QObject::connect(transaction.sendHush, &QPushButton::clicked, [&] (){
            
            QString amt = transaction.amountChat->text();
            QString memo = transaction.MemoMoney->text();
            this->setAmt(amt);
            this->setMoneyMemo(memo);          
            transactionDialog.close();
        });
        
        QObject::connect(transaction.sendHush, &QPushButton::clicked, this , &MainWindow::sendMoneyChat);


        
        QObject::connect(transaction.requestHush, &QPushButton::clicked, [&] (){
            
            QString amt = transaction.amountChat->text();
            QString memo = transaction.MemoMoney->text();
            this->setAmt(amt);
            this->setMoneyMemo(memo);          
            transactionDialog.close();
        });

        QObject::connect(transaction.requestHush, &QPushButton::clicked, this , &MainWindow::sendMoneyRequestChat);

        
             
        transactionDialog.exec();
        
     }); 

          QObject::connect(editAction, &QAction::triggered, [=]() {
          QModelIndex index = ui->listContactWidget->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();
        
        for(auto &p : AddressBook::getInstance()->getAllAddressLabels())
        if (label_contact == p.getName()) {
        
            QString label1 = p.getName();
            QString addr = p.getPartnerAddress();
            QString myzaddr =  p.getMyAddress();
            QString cid = p.getCid();
            QString avatar = p.getAvatar();


     AddressBook::getInstance()->removeAddressLabel(label1, addr, myzaddr, cid,avatar);
     rpc->refreshContacts(
            ui->listContactWidget);
     rpc->refresh(true);
        }    
     });

   
   
ui->memoTxtChat->setLenDisplayLabelChat(ui->memoSizeChat);

}

// Create a Tx from the current state of the Chat page. 
Tx MainWindow::createTxFromSendChatPage() {
   Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
  {
       
        QString amtStr = this->getAmt();
        CAmount amt; 
        CAmount amtHm;
       
            amt = CAmount::fromDecimalString(amtStr);
            amtHm = CAmount::fromDecimalString("0");
            totalAmt = totalAmt + amt;

        QModelIndex index = ui->listContactWidget->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();

    for(auto &c : AddressBook::getInstance()->getAllAddressLabels())

     if (label_contact == c.getName()) {
     
            QString cid = c.getCid();
            QString myAddr = c.getMyAddress();
            QString type = "Money";
            QString addr = c.getPartnerAddress();
            QString moneymemo = this->getMoneyMemo();
           
             /////////User input for chatmemos
        QString memoplain = QString("Money transaction of : ") + amtStr + QString(" HUSH") +  QString("\n") +  QString("\n") + moneymemo;

  /////////We convert the user input from QString to unsigned char*, so we can encrypt it later
        int lengthmemo = memoplain.length();

        char *memoplainchar = NULL;
        memoplainchar = new char[lengthmemo+2];
        strncpy(memoplainchar, memoplain.toUtf8(), lengthmemo +1);

        QString pubkey = this->getPubkeyByAddress(addr);
        QString passphraseHash = DataStore::getChatDataStore()->getPassword();
        int length = passphraseHash.length();

 ////////////////Generate the secretkey for our message encryption

        char *hashEncryptionKeyraw = NULL;
        hashEncryptionKeyraw = new char[length+1];
        strncpy(hashEncryptionKeyraw, passphraseHash.toUtf8(), length+1);

        #define MESSAGEAS1 ((const unsigned char *) hashEncryptionKeyraw)
        #define MESSAGEAS1_LEN length
    

        unsigned char sk[crypto_kx_SECRETKEYBYTES];
        unsigned char pk[crypto_kx_PUBLICKEYBYTES];
        unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
      
                if (crypto_kx_seed_keypair(pk,sk,
                           MESSAGEAS1) !=0) {

                               this->logger->write("Suspicious keypair, bail out ");
                           }
         ////////////////Get the pubkey from Bob, so we can create the share key

        const QByteArray pubkeyBobArray = QByteArray::fromHex(pubkey.toLatin1());
        const unsigned char *pubkeyBob = reinterpret_cast<const unsigned char *>(pubkeyBobArray.constData());
                    /////Create the shared key for sending the message

            if (crypto_kx_server_session_keys(server_rx, server_tx,
                                  pk, sk, pubkeyBob) != 0) {
            this->logger->write("Suspicious client public send key, bail out ");
             }

    
            // Let's try to preserve Unicode characters
            QByteArray ba_memo = memoplain.toUtf8();
            int ba_memo_length = ba_memo.size();

            #define MESSAGEMoney (const unsigned char *) ba_memo.data()
            #define MESSAGE_LENMoney ba_memo_length


    ////////////Now lets encrypt the message Alice send to Bob//////////////////////////////
             //#define MESSAGE (const unsigned char *) memoplainchar
             //#define MESSAGE_LEN lengthmemo
             #define CIPHERTEXT_LEN (crypto_secretstream_xchacha20poly1305_ABYTES + MESSAGE_LENMoney)
             unsigned char ciphertext[CIPHERTEXT_LEN];
             unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];

            crypto_secretstream_xchacha20poly1305_state state;

            /* Set up a new stream: initialize the state and create the header */
            crypto_secretstream_xchacha20poly1305_init_push(&state, header, server_tx);


             /* Now, encrypt the first chunk. `c1` will contain an encrypted,
            * authenticated representation of `MESSAGE_PART1`. */
            crypto_secretstream_xchacha20poly1305_push
            (&state, ciphertext, NULL, MESSAGEMoney, MESSAGE_LENMoney, NULL, 0, crypto_secretstream_xchacha20poly1305_TAG_FINAL);

            ////Create the HM for this message
            QString headerbytes = QByteArray(reinterpret_cast<const char*>(header), crypto_secretstream_xchacha20poly1305_HEADERBYTES).toHex();
            QString publickeyAlice = QByteArray(reinterpret_cast<const char*>(pk), crypto_kx_PUBLICKEYBYTES).toHex();


            QString hmemo= createHeaderMemo(type,cid,myAddr,headerbytes,publickeyAlice,1,0);

             /////Ciphertext Memo
            QString memo = QByteArray(reinterpret_cast<const char*>(ciphertext), CIPHERTEXT_LEN).toHex();
         
   
             tx.toAddrs.push_back(ToFields{addr, amtHm, hmemo});
             tx.toAddrs.push_back(ToFields{addr, amt, memo});

   } 
   }

    tx.fee = Settings::getMinerFee();

     return tx;

}

void MainWindow::sendMoneyChat() {

////////////////////////////Todo: Check if a Contact is selected//////////

    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.
 
  /* QString Name = ui->contactNameMemo->text();

      if ((ui->contactNameMemo->text().isEmpty()) || (ui->memoTxtChat->toPlainText().trimmed().isEmpty())) {
     
        QMessageBox msg(QMessageBox::Critical, tr("You have to select a contact and insert a Memo"),
        tr("You have selected no Contact from Contactlist,\n")  + tr("\nor your Memo is empty"),
        QMessageBox::Ok, this);

        msg.exec();
        return;
    }*/


    Tx tx = createTxFromSendChatPage();

    QString error = doSendChatMoneyTxValidations(tx);

    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(QMessageBox::Critical, tr("Message Error"), error,
                        QMessageBox::Ok, this);

        msg.exec();
        ui->memoTxtChat->setEnabled(true);

        // abort the Tx
        return;
    }

        auto movie = new QMovie(this);
        auto movie1 = new QMovie(this);
        movie->setFileName(":/img/res/loaderblack.gif");
        movie1->setFileName(":/img/res/loaderwhite.gif");
     
        auto theme = Settings::getInstance()->get_theme_name();
        if (theme == "Dark" || theme == "Midnight") {

        connect(movie, &QMovie::frameChanged, [=]{
        ui->sendChatButton->setIcon(movie->currentPixmap());
        });      
        movie->start();
        ui->sendChatButton->show();
        ui->sendChatButton->setEnabled(false);
             
        } else {

        connect(movie1, &QMovie::frameChanged, [=]{
        ui->sendChatButton->setIcon(movie1->currentPixmap());
        });      
        movie1->start();
        ui->sendChatButton->show();
        ui->sendChatButton->setEnabled(false);
        ui->memoTxtChat->setEnabled(false);
        }

        ui->memoTxtChat->clear();
        
        // And send the Tx
        rpc->executeTransaction(tx, 
            [=] (QString txid) { 
                ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
                

            QTimer::singleShot(1000, [=]() {
         
            if (theme == "Dark" || theme == "Midnight") {
            QPixmap send(":/icons/res/send-white.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie->stop();
            ui->sendChatButton->setEnabled(true);
            ui->memoTxtChat->setEnabled(true);
             }else{
            
            QPixmap send(":/icons/res/sendBlack.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie1->stop();
            ui->sendChatButton->setEnabled(true);
            ui->memoTxtChat->setEnabled(true);
             }
                    
                  });
                
                // Force a UI update so we get the unconfirmed Tx
                rpc->refresh(true);
                ui->memoTxtChat->clear();
               // ui->memoTxtChat->setEnabled(true);

            },
            // Errored out
            [=] (QString opid, QString errStr) {
                ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
                ui->memoTxtChat->setEnabled(true);
                
                if (!opid.isEmpty())
                    errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            

                QMessageBox::critical(this, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);
                         movie->stop();
                         ui->memoTxtChat->setEnabled(true);
      
              
            if (theme == "Dark" || theme == "Midnight") {
            QPixmap send(":/icons/res/send-white.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie->stop();
            ui->sendChatButton->setEnabled(true);
            ui->memoTxtChat->setEnabled(true);
             }else{
            
            QPixmap send(":/icons/res/sendBlack.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie1->stop();
            ui->sendChatButton->setEnabled(true);
            ui->memoTxtChat->setEnabled(true);
             }
                                    
                           
            }
        );

    }        

QString MainWindow::doSendChatMoneyTxValidations(Tx tx) {
    // Check to see if we have enough verified funds to send the Tx.

    CAmount total;
    for (auto toAddr : tx.toAddrs) {
        if (!Settings::isValidAddress(toAddr.addr)) {
            QString addr = (toAddr.addr.length() > 100 ? toAddr.addr.left(100) + "..." : toAddr.addr);
            return QString(tr("Recipient Address ")) % addr % tr(" is Invalid");
            ui->memoTxtChat->setEnabled(true);
        }

        // This technically shouldn't be possible, but issue #62 seems to have discovered a bug
        // somewhere, so just add a check to make sure. 
        if (toAddr.amount.toqint64() < 0) {
            return QString(tr("Amount for address '%1' is invalid!").arg(toAddr.addr));
             ui->memoTxtChat->setEnabled(true);
        }

        total = total + toAddr.amount;
    }
    total = total + tx.fee;

    auto available = rpc->getModel()->getAvailableBalance();

    if (available < total) {
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 1 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
            ui->memoTxtChat->setEnabled(true);

    }

    return "";
}

// Create a Tx from the current state of the Chat page. 
Tx MainWindow::createTxFromSendRequestChatPage() {
   Tx tx;
    CAmount totalAmt;
    // For each addr/amt in the Chat tab
  {
       
        QString amtStr = this->getAmt();
        CAmount amt; 
        CAmount amtHm;
       
            amt = CAmount::fromDecimalString("0");
            amtHm = CAmount::fromDecimalString("0");
            totalAmt = totalAmt + amt;

        QModelIndex index = ui->listContactWidget->currentIndex();
        QString label_contact = index.data(Qt::DisplayRole).toString();

    for(auto &c : AddressBook::getInstance()->getAllAddressLabels())

     if (label_contact == c.getName()) {
     
            QString cid = c.getCid();
            QString myAddr = c.getMyAddress();
            QString type = "Money";
            QString addr = c.getPartnerAddress();
            QString moneymemo = this->getMoneyMemo();
           
             /////////User input for chatmemos
        QString memoplain = QString("Request of : ") + amtStr + QString(" HUSH ") +  QString("\n") +  QString("\n") + moneymemo;

  /////////We convert the user input from QString to unsigned char*, so we can encrypt it later
        int lengthmemo = memoplain.length();

        char *memoplainchar = NULL;
        memoplainchar = new char[lengthmemo+2];
        strncpy(memoplainchar, memoplain.toUtf8(), lengthmemo +1);

        QString pubkey = this->getPubkeyByAddress(addr);
        QString passphraseHash = DataStore::getChatDataStore()->getPassword();
        int length = passphraseHash.length();

 ////////////////Generate the secretkey for our message encryption

        char *hashEncryptionKeyraw = NULL;
        hashEncryptionKeyraw = new char[length+1];
        strncpy(hashEncryptionKeyraw, passphraseHash.toUtf8(), length+1);

        #define MESSAGEAS1 ((const unsigned char *) hashEncryptionKeyraw)
        #define MESSAGEAS1_LEN length
    

        unsigned char sk[crypto_kx_SECRETKEYBYTES];
        unsigned char pk[crypto_kx_PUBLICKEYBYTES];
        unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
      
                if (crypto_kx_seed_keypair(pk,sk,
                           MESSAGEAS1) !=0) {

                               this->logger->write("Suspicious keypair, bail out ");
                           }
         ////////////////Get the pubkey from Bob, so we can create the share key

        const QByteArray pubkeyBobArray = QByteArray::fromHex(pubkey.toLatin1());
        const unsigned char *pubkeyBob = reinterpret_cast<const unsigned char *>(pubkeyBobArray.constData());
                    /////Create the shared key for sending the message

            if (crypto_kx_server_session_keys(server_rx, server_tx,
                                  pk, sk, pubkeyBob) != 0) {
            this->logger->write("Suspicious client public send key, bail out ");
             }

    
            // Let's try to preserve Unicode characters
            QByteArray ba_memo = memoplain.toUtf8();
            int ba_memo_length = ba_memo.size();

            #define MESSAGEMoney (const unsigned char *) ba_memo.data()
            #define MESSAGE_LENMoney ba_memo_length


    ////////////Now lets encrypt the message Alice send to Bob//////////////////////////////
             //#define MESSAGE (const unsigned char *) memoplainchar
             //#define MESSAGE_LEN lengthmemo
             #define CIPHERTEXT_LEN (crypto_secretstream_xchacha20poly1305_ABYTES + MESSAGE_LENMoney)
             unsigned char ciphertext[CIPHERTEXT_LEN];
             unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];

            crypto_secretstream_xchacha20poly1305_state state;

            /* Set up a new stream: initialize the state and create the header */
            crypto_secretstream_xchacha20poly1305_init_push(&state, header, server_tx);


             /* Now, encrypt the first chunk. `c1` will contain an encrypted,
            * authenticated representation of `MESSAGE_PART1`. */
            crypto_secretstream_xchacha20poly1305_push
            (&state, ciphertext, NULL, MESSAGEMoney, MESSAGE_LENMoney, NULL, 0, crypto_secretstream_xchacha20poly1305_TAG_FINAL);

            ////Create the HM for this message
            QString headerbytes = QByteArray(reinterpret_cast<const char*>(header), crypto_secretstream_xchacha20poly1305_HEADERBYTES).toHex();
            QString publickeyAlice = QByteArray(reinterpret_cast<const char*>(pk), crypto_kx_PUBLICKEYBYTES).toHex();


            QString hmemo= createHeaderMemo(type,cid,myAddr,headerbytes,publickeyAlice,1,0);

             /////Ciphertext Memo
            QString memo = QByteArray(reinterpret_cast<const char*>(ciphertext), CIPHERTEXT_LEN).toHex();
         
   
             tx.toAddrs.push_back(ToFields{addr, amtHm, hmemo});
             tx.toAddrs.push_back(ToFields{addr, amt, memo});

   } 
   }

    tx.fee = Settings::getMinerFee();

     return tx;

}

void MainWindow::sendMoneyRequestChat() {

////////////////////////////Todo: Check if a Contact is selected//////////

    // Create a Tx from the values on the send tab. Note that this Tx object
    // might not be valid yet.
 
  /* QString Name = ui->contactNameMemo->text();

      if ((ui->contactNameMemo->text().isEmpty()) || (ui->memoTxtChat->toPlainText().trimmed().isEmpty())) {
     
        QMessageBox msg(QMessageBox::Critical, tr("You have to select a contact and insert a Memo"),
        tr("You have selected no Contact from Contactlist,\n")  + tr("\nor your Memo is empty"),
        QMessageBox::Ok, this);

        msg.exec();
        return;
    }*/


    Tx tx = createTxFromSendRequestChatPage();

    QString error = doSendChatMoneyRequestTxValidations(tx);

    if (!error.isEmpty()) {
        // Something went wrong, so show an error and exit
        QMessageBox msg(QMessageBox::Critical, tr("Message Error"), error,
                        QMessageBox::Ok, this);

        msg.exec();

        // abort the Tx
        return;
    }

        auto movie = new QMovie(this);
        auto movie1 = new QMovie(this);
        movie->setFileName(":/img/res/loaderblack.gif");
        movie1->setFileName(":/img/res/loaderwhite.gif");
     
        auto theme = Settings::getInstance()->get_theme_name();
        if (theme == "Dark" || theme == "Midnight") {

        connect(movie, &QMovie::frameChanged, [=]{
        ui->sendChatButton->setIcon(movie->currentPixmap());
        });      
        movie->start();
        ui->sendChatButton->show();
        ui->sendChatButton->setEnabled(false);
             
        } else {

        connect(movie1, &QMovie::frameChanged, [=]{
        ui->sendChatButton->setIcon(movie1->currentPixmap());
        });      
        movie1->start();
        ui->sendChatButton->show();
        ui->sendChatButton->setEnabled(false);
        }

        ui->memoTxtChat->clear();
        
        // And send the Tx
        rpc->executeTransaction(tx, 
            [=] (QString txid) { 
                ui->statusBar->showMessage(Settings::txidStatusMessage + " " + txid);
                

            QTimer::singleShot(1000, [=]() {
         
            if (theme == "Dark" || theme == "Midnight") {
            QPixmap send(":/icons/res/send-white.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie->stop();
            ui->sendChatButton->setEnabled(true);
             }else{
            
            QPixmap send(":/icons/res/sendBlack.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie1->stop();
            ui->sendChatButton->setEnabled(true);
             }
                    
                  });
                
                // Force a UI update so we get the unconfirmed Tx
                rpc->refresh(true);
                ui->memoTxtChat->clear();

            },
            // Errored out
            [=] (QString opid, QString errStr) {
                ui->statusBar->showMessage(QObject::tr(" Tx ") % opid % QObject::tr(" failed"), 15 * 1000);
                
                if (!opid.isEmpty())
                    errStr = QObject::tr("The transaction with id ") % opid % QObject::tr(" failed. The error was") + ":\n\n" + errStr;            

                QMessageBox::critical(this, QObject::tr("Transaction Error"), errStr, QMessageBox::Ok);
                         movie->stop();
      
              
            if (theme == "Dark" || theme == "Midnight") {
            QPixmap send(":/icons/res/send-white.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie->stop();
            ui->sendChatButton->setEnabled(true);
             }else{
            
            QPixmap send(":/icons/res/sendBlack.png");
            QIcon sendIcon(send);
            ui->sendChatButton->setIcon(sendIcon);
            movie1->stop();
            ui->sendChatButton->setEnabled(true);
             }
                    
                   
                           
            }
        );

    }        

QString MainWindow::doSendChatMoneyRequestTxValidations(Tx tx) {
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
        return tr("Not enough available funds to send this transaction\n\nHave: %1\nNeed: %2\n\nNote: Funds need 1 confirmations before they can be spent")
            .arg(available.toDecimalhushString(), total.toDecimalhushString());
    }

    return "";
}


void MainWindow::updateChat()
{
    rpc->refreshChat(ui->listChat,ui->memoSizeChat);
    rpc->refresh(true);
}

void MainWindow::updateContacts()
{
    
}

void MainWindow::addNewZaddr(bool sapling) {
    rpc->createNewZaddr(sapling, [=] (json reply) {
        QString addr = QString::fromStdString(reply.get<json::array_t>()[0]);
        // Make sure the RPC class reloads the z-addrs for future use
        rpc->refreshAddresses();

        // Just double make sure the z-address is still checked
        if ( sapling && ui->rdioZSAddr->isChecked() ) {
            ui->listReceiveAddresses->insertItem(0, addr); 
            ui->listReceiveAddresses->setCurrentIndex(0);

            ui->statusBar->showMessage(QString::fromStdString("Created new zAddr") %
                                       (sapling ? "(Sapling)" : "(Sprout)"), 
                                       10 * 1000);
        }
    });
}


// Adds sapling or sprout z-addresses to the combo box. Technically, returns a
// lambda, which can be connected to the appropriate signal
std::function<void(bool)> MainWindow::addZAddrsToComboList(bool sapling) {
    return [=] (bool checked) { 
        if (checked) { 
            auto addrs = this->rpc->getModel()->getAllZAddresses();

            // Save the current address, so we can update it later
            auto zaddr = ui->listReceiveAddresses->currentText();
            ui->listReceiveAddresses->clear();

            std::for_each(addrs.begin(), addrs.end(), [=] (auto addr) {
                if ( (sapling &&  Settings::getInstance()->isSaplingAddress(addr)) ||
                    (!sapling && !Settings::getInstance()->isSaplingAddress(addr))) {                        
                        auto bal = rpc->getModel()->getAllBalances().value(addr);
                        ui->listReceiveAddresses->addItem(addr, bal);
                }
            }); 
            
            if (!zaddr.isEmpty() && Settings::isZAddress(zaddr)) {
                ui->listReceiveAddresses->setCurrentText(zaddr);
            }

            // If z-addrs are empty, then create a new one.
            if (addrs.isEmpty()) {
                addNewZaddr(sapling);
            }
        } 
    };
}

void MainWindow::setupReceiveTab() {
    auto addNewTAddr = [=] () {
        rpc->createNewTaddr([=] (json reply) {
            QString addr = QString::fromStdString(reply.get<json::array_t>()[0]);
            // Make sure the RPC class reloads the t-addrs for future use
            rpc->refreshAddresses();

            // Just double make sure the t-address is still checked
            if (ui->rdioTAddr->isChecked()) {
                ui->listReceiveAddresses->insertItem(0, addr);
                ui->listReceiveAddresses->setCurrentIndex(0);

                ui->statusBar->showMessage(tr("Created new t-Addr"), 10 * 1000);
            }
        });
    };

    // Connect t-addr radio button
    QObject::connect(ui->rdioTAddr, &QRadioButton::toggled, [=] (bool checked) { 
        // Whenever the t-address is selected, we generate a new address, because we don't
        // want to reuse t-addrs
        if (checked) { 
            updateTAddrCombo(checked);
        } 


    });

    // View all addresses goes to "View all private keys"
    QObject::connect(ui->btnViewAllAddresses, &QPushButton::clicked, [=] () {
        // If there's no RPC, return
        if (!getRPC())
            return;

        QDialog d(this);
        Ui_ViewAddressesDialog viewaddrs;
        viewaddrs.setupUi(&d);
        Settings::saveRestore(&d);
        Settings::saveRestoreTableHeader(viewaddrs.tblAddresses, &d, "viewalladdressestable");
        viewaddrs.tblAddresses->horizontalHeader()->setStretchLastSection(true);

         QList<QString> allAddresses;
        if (ui->rdioTAddr->isChecked()) {
            allAddresses = getRPC()->getModel()->getAllTAddresses();
        } else {
            allAddresses = getRPC()->getModel()->getAllZAddresses();
        }

        ViewAllAddressesModel model(viewaddrs.tblAddresses, allAddresses, getRPC());
        viewaddrs.tblAddresses->setModel(&model);

        QObject::connect(viewaddrs.btnExportAll, &QPushButton::clicked,  this, &MainWindow::exportAllKeys);

        viewaddrs.tblAddresses->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(viewaddrs.tblAddresses, &QTableView::customContextMenuRequested, [=] (QPoint pos) {
            QModelIndex index = viewaddrs.tblAddresses->indexAt(pos);
            if (index.row() < 0) return;

            index = index.sibling(index.row(), 0);
            QString addr = viewaddrs.tblAddresses->model()->data(index).toString();

            QMenu menu(this);
            menu.addAction(tr("Export Private Key"), [=] () {                
                if (addr.isEmpty())
                    return;

                this->exportKeys(addr);
            });
            menu.addAction(tr("Copy Address"), [=]() {
                QGuiApplication::clipboard()->setText(addr);
            });
            menu.exec(viewaddrs.tblAddresses->viewport()->mapToGlobal(pos));
        });

        d.exec();
    });

    QObject::connect(ui->rdioZSAddr, &QRadioButton::toggled, addZAddrsToComboList(true));

    // Explicitly get new address button.
    QObject::connect(ui->btnReceiveNewAddr, &QPushButton::clicked, [=] () {
        if (!rpc->getConnection())
            return;
        
                // Go over the dropdown and just select the next address that has:
        // 0 balance and has no labels
        for (int i=ui->listReceiveAddresses->currentIndex()+1; i < ui->listReceiveAddresses->count(); i++) {
            QString item = ui->listReceiveAddresses->itemText(i);
            CAmount bal = getRPC()->getModel()->getAllBalances().value(item, CAmount());
            if (bal == 0 && AddressBook::getInstance()->getLabelForAddress(item).isEmpty()) {
                // Pick this one, since it has no balance and no label
                ui->listReceiveAddresses->setCurrentIndex(i);
                return;
            }
        }

        // If none of the existing items were eligible, create a new one.

        if (ui->rdioZSAddr->isChecked()) {
            addNewZaddr(true);
        } else if (ui->rdioTAddr->isChecked()) {
            addNewTAddr();
        }
    });

    // Focus enter for the Receive Tab
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, [=] (int tab) {
        if (tab == 2) {
            // Switched to receive tab, select the z-addr radio button
            ui->rdioZSAddr->setChecked(true);
            
            
            // And then select the first one
            ui->listReceiveAddresses->setCurrentIndex(0);
        }
    });

    // Validator for label
    QRegExpValidator* v = new QRegExpValidator(QRegExp(Settings::labelRegExp), ui->rcvLabel);
    ui->rcvLabel->setValidator(v);

    // Select item in address list
    QObject::connect(ui->listReceiveAddresses, 
        QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (int index) {
        QString addr = ui->listReceiveAddresses->itemText(index);
        if (addr.isEmpty()) {
            // Draw empty stuff

            ui->rcvLabel->clear();
            ui->rcvBal->clear();
            ui->txtReceive->clear();
            ui->qrcodeDisplay->clear();
            return;
        }

        auto label = AddressBook::getInstance()->getLabelForAddress(addr);
        if (label.isEmpty()) {
            ui->rcvUpdateLabel->setText("Add Label");
        }
        else {
            ui->rcvUpdateLabel->setText("Update Label");
        }
        
        ui->rcvLabel->setText(label);
        if (Settings::getInstance()->get_currency_name() == "USD") {
             ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushUSDString());
        } else if (Settings::getInstance()->get_currency_name() == "EUR") {
             ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushEURString());
        } else if (Settings::getInstance()->get_currency_name() == "BTC") {
             ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushBTCString());
        } else if (Settings::getInstance()->get_currency_name() == "CNY") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushCNYString());
        } else if (Settings::getInstance()->get_currency_name() == "RUB") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushRUBString());
        } else if (Settings::getInstance()->get_currency_name() == "CAD") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushCADString());
        } else if (Settings::getInstance()->get_currency_name() == "SGD") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushSGDString());
        } else if (Settings::getInstance()->get_currency_name() == "CHF") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushCHFString());
        } else if (Settings::getInstance()->get_currency_name() == "INR") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushINRString());
        } else if (Settings::getInstance()->get_currency_name() == "GBP") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushGBPString());
        } else if (Settings::getInstance()->get_currency_name() == "AUD") {
            ui->rcvBal->setText(rpc->getModel()->getAllBalances().value(addr).toDecimalhushAUDString());
            }
        
        ui->txtReceive->setPlainText(addr);       
        ui->qrcodeDisplay->setQrcodeString(addr);
        if (rpc->getModel()->getUsedAddresses().value(addr, false)) {
            ui->rcvBal->setToolTip(tr("Address has been previously used"));
        } else {
            ui->rcvBal->setToolTip(tr("Address is unused"));
        }
        
    });    

    // Receive tab add/update label
    QObject::connect(ui->rcvUpdateLabel, &QPushButton::clicked, [=]() {
        QString addr = ui->listReceiveAddresses->currentText();
       
        if (addr.isEmpty())
            return;

        auto curLabel = AddressBook::getInstance()->getLabelForAddress(addr);
        auto label = ui->rcvLabel->text().trimmed();

        if (curLabel == label)  // Nothing to update
            return;

        QString info;

        if (!curLabel.isEmpty() && label.isEmpty()) {
            info = "Removed Label '" % curLabel % "'";
            AddressBook::getInstance()->removeAddressLabel(curLabel, addr, "", "","" );
        }
        else if (!curLabel.isEmpty() && !label.isEmpty()) {
            info = "Updated Label '" % curLabel % "' to '" % label % "'";
            AddressBook::getInstance()->updateLabel(curLabel, addr, label);
        }
        else if (curLabel.isEmpty() && !label.isEmpty()) {
            info = "Added Label '" % label % "'";
            AddressBook::getInstance()->addAddressLabel(label, addr, "", "", "");
        }

        // Update labels everywhere on the UI
        updateLabels();

        // Show the user feedback
        if (!info.isEmpty()) {
            QMessageBox::information(this, "Label", info, QMessageBox::Ok);
        }
    });

    // Receive Export Key
    QObject::connect(ui->exportKey, &QPushButton::clicked, [=]() {
        QString addr = ui->listReceiveAddresses->currentText();
        if (addr.isEmpty())
            return;

        this->exportKeys(addr);
    });
}

void MainWindow::updateTAddrCombo(bool checked) {
    if (checked) {
        auto utxos = this->rpc->getModel()->getUTXOs();

        // Save the current address so we can restore it later
        auto currentTaddr = ui->listReceiveAddresses->currentText();

        ui->listReceiveAddresses->clear();

        // Maintain a set of addresses so we don't duplicate any, because we'll be adding
        // t addresses multiple times
        QSet<QString> addrs;

        // 1. Add all t addresses that have a balance
        std::for_each(utxos.begin(), utxos.end(), [=, &addrs](auto& utxo) {
            auto addr = utxo.address;
            if (Settings::isTAddress(addr) && !addrs.contains(addr)) {
                auto bal = rpc->getModel()->getAllBalances().value(addr);
                ui->listReceiveAddresses->addItem(addr, bal);

                addrs.insert(addr);
            }
        });
        
        // 2. Add all t addresses that have a label
        auto allTaddrs = this->rpc->getModel()->getAllTAddresses();
        QSet<QString> labels;
        for (auto p : AddressBook::getInstance()->getAllAddressLabels()) {
            labels.insert(p.getPartnerAddress());
        }
        std::for_each(allTaddrs.begin(), allTaddrs.end(), [=, &addrs] (auto& taddr) {
            // If the address is in the address book, add it. 
            if (labels.contains(taddr) && !addrs.contains(taddr)) {
                addrs.insert(taddr);
                ui->listReceiveAddresses->addItem(taddr, CAmount::fromqint64(0));
            }
        });

        // 3. Add all t-addresses. We won't add more than 20 total t-addresses,
        // since it will overwhelm the dropdown
        for (int i=0; addrs.size() < 20 && i < allTaddrs.size(); i++) {
            auto addr = allTaddrs.at(i);
            if (!addrs.contains(addr))  {
                addrs.insert(addr);
                // Balance is zero since it has not been previously added
                ui->listReceiveAddresses->addItem(addr, CAmount::fromqint64(0));
            }
        }

        // 4. Add the previously selected t-address
        if (!currentTaddr.isEmpty() && Settings::isTAddress(currentTaddr)) {
            // Make sure the current taddr is in the list
            if (!addrs.contains(currentTaddr)) {
                auto bal = rpc->getModel()->getAllBalances().value(currentTaddr);
                ui->listReceiveAddresses->addItem(currentTaddr, bal);
            }
            ui->listReceiveAddresses->setCurrentText(currentTaddr);
        }

        // 5. Add a last, disabled item if there are remaining items
        if (allTaddrs.size() > addrs.size()) {
            auto num = QString::number(allTaddrs.size() - addrs.size());
            ui->listReceiveAddresses->addItem("-- " + num + " more --", CAmount::fromqint64(0));

            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->listReceiveAddresses->model());
            QStandardItem* item =  model->findItems("--", Qt::MatchStartsWith)[0];
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
};

// Updates the labels everywhere on the UI. Call this after the labels have been updated
void MainWindow::updateLabels() {
    // Update the Receive tab
    if (ui->rdioTAddr->isChecked()) {
        updateTAddrCombo(true);
    }
    else {
        addZAddrsToComboList(ui->rdioZSAddr->isChecked())(true);
    }

    // Update the autocomplete
    updateLabelsAutoComplete();
}

void MainWindow::slot_change_currency(const QString& currency_name)

{
    
    Settings::getInstance()->set_currency_name(currency_name);

    // Include currency

    QString saved_currency_name;
    try
    {
       saved_currency_name = Settings::getInstance()->get_currency_name();
       
    }
    catch (...)
    {
        saved_currency_name = "USD";
        
    }
}

void MainWindow::slot_change_theme(const QString& theme_name)

{
    Settings::getInstance()->set_theme_name(theme_name);
    

    // Include css
    QString saved_theme_name;
    try
    {
       saved_theme_name = Settings::getInstance()->get_theme_name();
    }
    catch (...)
    {
        saved_theme_name = "Dark";
    }

    QFile qFile(":/css/res/css/" + saved_theme_name +".css");
    if (qFile.open(QFile::ReadOnly))
    {
      QString styleSheet = QLatin1String(qFile.readAll());
      this->setStyleSheet(""); // resets styles, makes app restart unnecessary
      this->setStyleSheet(styleSheet);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
    delete rpc;
    delete labelCompleter;

    delete sendTxRecurringInfo;
    delete amtValidator;
    delete feesValidator;

    delete loadingMovie;
    delete logger;

    delete wsserver;
    delete wormhole;
}
void MainWindow::on_givemeZaddr_clicked()
{

    bool sapling = true;
    rpc->createNewZaddr(sapling, [=] (json reply) {
                QString hushchataddr = QString::fromStdString(reply.get<json::array_t>()[0]);
                QClipboard *zaddr_Clipboard = QApplication::clipboard();
                zaddr_Clipboard ->setText(hushchataddr);
                QMessageBox::information(this, "Your new HushChat address was copied to your clipboard!",hushchataddr);
                ui->listReceiveAddresses->insertItem(0, hushchataddr);
                ui->listReceiveAddresses->setCurrentIndex(0);
              
                });
}

void MainWindow::on_emojiButton_clicked()
{

        QDialog emojiDialog(this);
        Ui_emojiDialog emoji;
        emoji.setupUi(&emojiDialog);
        Settings::saveRestore(&emojiDialog);

QObject::connect(emoji.smiley, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":smiley:");

        emojiDialog.close();
});

QObject::connect(emoji.money, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":money_mouth:");

        emojiDialog.close();
});

QObject::connect(emoji.laughing, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":laughing:");

        emojiDialog.close();
});

QObject::connect(emoji.sweet_smile, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":sweet_smile:");

        emojiDialog.close();
});

QObject::connect(emoji.joy, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":joy:");

        emojiDialog.close();
});

QObject::connect(emoji.innocent, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":innocent:");

        emojiDialog.close();
});

QObject::connect(emoji.partying_face, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":partying_face:");

        emojiDialog.close();
});

QObject::connect(emoji.rolling_eyes, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":rolling_eyes:");

        emojiDialog.close();
});

QObject::connect(emoji.tongue, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":stuck_out_tongue:");

        emojiDialog.close();
});

QObject::connect(emoji.hearts3, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":face_with_3hearts:");

        emojiDialog.close();
});

QObject::connect(emoji.heart_eyes, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":heart_eyes:");

        emojiDialog.close();
});

QObject::connect(emoji.nauseated, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":nauseated:");

        emojiDialog.close();
});

QObject::connect(emoji.poop, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":poop:");

        emojiDialog.close();
});

QObject::connect(emoji.symbols_mouth, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":symbols_mouth:");

        emojiDialog.close();
});

QObject::connect(emoji.sunglass, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":sunglass:");

        emojiDialog.close();
});

QObject::connect(emoji.stuck_out, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":stuck_out:");

        emojiDialog.close();
});

QObject::connect(emoji.hush_white, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":hush_white:");

        emojiDialog.close();
});

QObject::connect(emoji.sd, &QPushButton::clicked, [&] () {
   ui->memoTxtChat->insertHtml(":sd:");

        emojiDialog.close();
});



    emojiDialog.exec();
}

