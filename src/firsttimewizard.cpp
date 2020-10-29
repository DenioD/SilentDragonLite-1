#include "firsttimewizard.h"

#include "ui_newseed.h"
#include "ui_restoreseed.h"
#include "ui_verifyseed.h"
#include "ui_newwallet.h"
#include "mainwindow.h"
#include "DataStore/DataStore.h"

#include "../lib/silentdragonlitelib.h"

#ifdef Q_OS_WIN
auto dirwalletfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletencfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackupfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.datBackup");
#endif
#ifdef Q_OS_MACOS
auto dirwalletfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletencfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackupfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.datBackup");
#endif
#ifdef Q_OS_LINUX
auto dirwalletfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet.dat");
auto dirwalletencfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet-enc.dat");
auto dirwalletbackupfirst = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet.datBackup");
#endif




void FirstTimeWizard::slot_change_theme(const QString& theme_name)

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

FirstTimeWizard::FirstTimeWizard(bool dangerous, QString server)
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

    setWindowTitle("New wallet wizard");
    this->dangerous = dangerous;
    this->server = server;


    ////backup addresslabels.dat if there is one, to restore it later

    auto dir =  QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QString addressbook = dir.filePath("addresslabels.dat.enc");
    QFile file(addressbook);

    if (file.exists())
    {
    file.rename(dir.filePath("addresslabels.dat.enc-backup"));

    }

    // Create the pages
    setPage(Page_NewOrRestore, new NewOrRestorePage(this));
    setPage(Page_New, new NewSeedPage(this));
    setPage(Page_Restore,new RestoreSeedPage(this));
}

int FirstTimeWizard::nextId() const {
    switch (currentId()) {
        case Page_NewOrRestore:
            if (field("intro.new").toBool()) {
                return Page_New;
            } else {
                return Page_Restore;
            }            
        case Page_New:
        case Page_Restore:
        default:
            return -1;
    }
}

QString FirstTimeWizard::getSeed()
{

    return _seed;
}

void FirstTimeWizard::setSeed(QString seed)
{

    _seed = seed;
}

QString FirstTimeWizard::getBirthday()
{

    return _birthday;
}

void FirstTimeWizard::setBirthday(QString birthday)
{

    _birthday = birthday;
}

NewOrRestorePage::NewOrRestorePage(FirstTimeWizard *parent) : QWizardPage(parent) {
    setTitle("Create or Restore wallet.");

    QWidget* pageWidget = new QWidget();
    Ui_CreateWalletForm form;
    form.setupUi(pageWidget);

    QGraphicsScene* scene = new QGraphicsScene();
    QGraphicsView* view = new QGraphicsView(scene);
    form.Logo->setScene(scene);
    QPixmap pixmap(":/icons/res/dark-01.png");
    scene->addPixmap(pixmap);
    form.Logo->show();


    parent->button(QWizard::CommitButton)->setEnabled(false);
    setButtonText(QWizard::CommitButton, "Next");
    form.txtPassword->setEnabled(false);
    form.txtConfirmPassword->setEnabled(false); 

           QObject::connect(form.TOS,  &QRadioButton::clicked, [=](bool checked) {
        if (checked) {

            form.txtPassword->setEnabled(true);
            form.txtConfirmPassword->setEnabled(true);                 
            
        }else{
            parent->button(QWizard::CommitButton)->setEnabled(false);
            parent->button(QWizard::NextButton)->setEnabled(false);
        }
    });


         auto fnPasswordEdited = [=](const QString&) {
        // Enable the Finish button if the passwords match.
        QString passphraseBlank = form.txtPassword->text();
  
        QString passphrase = QString("HUSH3") + passphraseBlank + QString("SDL");

        
        if (!form.txtPassword->text().isEmpty() && 
                form.txtPassword->text() == form.txtConfirmPassword->text() && passphraseBlank.size() >= 16 ){

            form.lblPasswordMatch->setText("");
            
            
            form.radioRestoreWallet->setEnabled(true);
            form.radioNewWallet->setEnabled(true);
            form.radioNewWallet->setChecked(true);
            parent->button(QWizard::NextButton)->setEnabled(false);
            

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
 
     // Exclusive buttons
    QObject::connect(form.radioNewWallet,  &QRadioButton::clicked, [=](bool checked) {
        if (checked) {
            form.radioRestoreWallet->setChecked(false);
            parent->button(QWizard::CommitButton)->setEnabled(true);
            
        }
    });

    QObject::connect(form.radioRestoreWallet, &QRadioButton::clicked, [=](bool checked) {
        if (checked) {
            form.radioNewWallet->setChecked(false);
            parent->button(QWizard::CommitButton)->setEnabled(true);
          
        }
    });

  
           

            
        } else {
            form.lblPasswordMatch->setText(tr("Passphrase don't match or You have entered too few letters (16 minimum)"));
            
           parent->button(QWizard::CommitButton)->setEnabled(false);
           form.radioRestoreWallet->setEnabled(false);
           form.radioNewWallet->setEnabled(false);
        }

    };

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pageWidget);
    setLayout(layout);
    

    QObject::connect(form.txtConfirmPassword, &QLineEdit::textChanged, fnPasswordEdited);
    QObject::connect(form.txtPassword, &QLineEdit::textChanged, fnPasswordEdited);
    registerField("intro.new", form.radioNewWallet);
    form.radioRestoreWallet->setEnabled(false);
    form.radioNewWallet->setEnabled(false);
    setCommitPage(true);


    
    
}

NewSeedPage::NewSeedPage(FirstTimeWizard *parent) : QWizardPage(parent) {
    this->parent = parent;

    setTitle("Your new wallet");

    QWidget* pageWidget = new QWidget();
    form.setupUi(pageWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pageWidget);
    
    setLayout(layout);
}

void NewSeedPage::initializePage() {
    // Call the library to create a new wallet.

    char* resp = litelib_initialize_new(parent->dangerous,parent->server.toStdString().c_str());
    QString reply = litelib_process_response(resp);

    auto parsed = json::parse(reply.toStdString().c_str(), nullptr, false);
    if (parsed.is_discarded() || parsed.is_null() || parsed.find("seed") == parsed.end()) {
        form.txtSeed->setPlainText(tr("Error creating a wallet") + "\n" + reply);
    } else {
        QString birthday = QString::number(parsed["birthday"].get<json::number_unsigned_t>());
        QString seed = QString::fromStdString(parsed["seed"].get<json::string_t>());
        form.txtSeed->setPlainText(seed);
        parent->setSeed(seed);
        parent->setBirthday(birthday);
        form.birthday->setPlainText(birthday);
        parent->button(QWizard::CancelButton)->setEnabled(false);
        disconnect(parent->button(QWizard::CancelButton ), SIGNAL( clicked() ), parent, SLOT( reject() ) );
        connect(parent->button(QWizard::CancelButton ), SIGNAL( clicked() ), parent, SLOT( cancelEvent() ) );

    }


}

void FirstTimeWizard::cancelEvent()
    {
    	if( QMessageBox::question( this, ( "Quit Setup" ), ( "Setup is not complete yet. Are you sure you want to quit setup?" ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
    		// allow cancel
    		reject();
    	}
    }

// Will be called just before closing. Make sure we can save the seed in the wallet
// before we allow the page to be closed
bool NewSeedPage::validatePage() {

    Ui_verifyseed verifyseed;
    QDialog dialog(this);
    verifyseed.setupUi(&dialog);
    Settings::saveRestore(&dialog);

    form.birthday->setVisible(false);
    form.txtSeed->setVisible(false);
   

    QString seed = parent->getSeed();
    QString birthday = parent->getBirthday();
    QString word1 = seed.split(" ").at(13);
    verifyseed.word1->setText(word1);
    QString word2 = seed.split(" ").at(23);
    verifyseed.word2->setText(word2);
    QString word3 = seed.split(" ").at(11);
    verifyseed.word3->setText(word3);
    QString word4 = seed.split(" ").at(15);
    verifyseed.word4->setText(word4);
    QString word5 = seed.split(" ").at(20);
    verifyseed.word5->setText(word5);
    QString word6 = seed.split(" ").at(12);
    verifyseed.word6->setText(word6);
    QString word7 = seed.split(" ").at(16);
    verifyseed.word7->setText(word7);
    QString word8 = seed.split(" ").at(19);
    verifyseed.word8->setText(word8);
    QString word9 = seed.split(" ").at(21);
    verifyseed.word9->setText(word9);
    QString word10 = seed.split(" ").at(17);
    verifyseed.word10->setText(word10);
    QString word11 = seed.split(" ").at(14);
    verifyseed.word11->setText(word11);
    QString word12 = seed.split(" ").at(2);
    verifyseed.word12->setText(word12);
    QString word13 = seed.split(" ").at(5);
    verifyseed.word13->setText(word13);
    QString word14 = seed.split(" ").at(0);
    verifyseed.word14->setText(word14);
    QString word15 = seed.split(" ").at(10);
    verifyseed.word15->setText(word15);
    QString word16 = seed.split(" ").at(3);
    verifyseed.word16->setText(word16);
    QString word17 = seed.split(" ").at(6);
    verifyseed.word17->setText(word17);
    QString word18 = seed.split(" ").at(9);
    verifyseed.word18->setText(word18);
    QString word19 = seed.split(" ").at(22);
    verifyseed.word19->setText(word19);
    QString word20 = seed.split(" ").at(7);
    verifyseed.word20->setText(word20);
    QString word21 = seed.split(" ").at(4);
    verifyseed.word21->setText(word21);
    QString word22 = seed.split(" ").at(8);
    verifyseed.word22->setText(word22);
    QString word23 = seed.split(" ").at(18);
    verifyseed.word23->setText(word23);
    QString word24 = seed.split(" ").at(1);
    verifyseed.word24->setText(word24);

     QObject::connect(verifyseed.word1, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word1 + " ");
         verifyseed.word1->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word1->font();
         button.setStrikeOut(true);
         verifyseed.word1->setFont(button);
         verifyseed.word1->setEnabled(false);
     });
     QObject::connect(verifyseed.word2, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word2);
         verifyseed.word2->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word2->font();
         button.setStrikeOut(true);
         verifyseed.word2->setFont(button);
         verifyseed.word2->setEnabled(false);
     });
     QObject::connect(verifyseed.word3, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word3 + " ");
         verifyseed.word3->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word3->font();
         button.setStrikeOut(true);
         verifyseed.word3->setFont(button);
         verifyseed.word3->setEnabled(false);
     });
     QObject::connect(verifyseed.word4, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word4 + " ");
         verifyseed.word4->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word4->font();
         button.setStrikeOut(true);
         verifyseed.word4->setFont(button);
         verifyseed.word4->setEnabled(false);
     });
     QObject::connect(verifyseed.word5, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word5 + " ");
         verifyseed.word5->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word5->font();
         button.setStrikeOut(true);
         verifyseed.word5->setFont(button);
         verifyseed.word5->setEnabled(false);
     });
     QObject::connect(verifyseed.word6, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word6 + " ");
         verifyseed.word6->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word6->font();
         button.setStrikeOut(true);
         verifyseed.word6->setFont(button);
         verifyseed.word6->setEnabled(false);
     });
        QObject::connect(verifyseed.word7, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word7 + " ");
         verifyseed.word7->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word7->font();
         button.setStrikeOut(true);
         verifyseed.word7->setFont(button);
         verifyseed.word7->setEnabled(false);
     });
     QObject::connect(verifyseed.word8, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word8 + " ");
         verifyseed.word8->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word8->font();
         button.setStrikeOut(true);
         verifyseed.word8->setFont(button);
         verifyseed.word8->setEnabled(false);
     });
     QObject::connect(verifyseed.word9, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word9 + " ");
         verifyseed.word9->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word9->font();
         button.setStrikeOut(true);
         verifyseed.word9->setFont(button);
         verifyseed.word9->setEnabled(false);
     });
     QObject::connect(verifyseed.word10, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word10 + " ");
         verifyseed.word10->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word10->font();
         button.setStrikeOut(true);
         verifyseed.word10->setFont(button);
         verifyseed.word10->setEnabled(false);
     });
     QObject::connect(verifyseed.word11, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word11 + " ");
         verifyseed.word11->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word11->font();
         button.setStrikeOut(true);
         verifyseed.word11->setFont(button);
         verifyseed.word11->setEnabled(false);
     });
     QObject::connect(verifyseed.word12, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word12 + " ");
         verifyseed.word12->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word12->font();
         button.setStrikeOut(true);
         verifyseed.word12->setFont(button);
         verifyseed.word12->setEnabled(false);
     });
        QObject::connect(verifyseed.word13, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word13 + " ");
         verifyseed.word13->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word13->font();
         button.setStrikeOut(true);
         verifyseed.word13->setFont(button);
         verifyseed.word13->setEnabled(false);
     });
     QObject::connect(verifyseed.word14, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word14 + " ");
         verifyseed.word14->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word14->font();
         button.setStrikeOut(true);
         verifyseed.word14->setFont(button);
         verifyseed.word14->setEnabled(false);
     });
     QObject::connect(verifyseed.word15, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word15 + " ");
         verifyseed.word15->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word15->font();
         button.setStrikeOut(true);
         verifyseed.word15->setFont(button);
         verifyseed.word15->setEnabled(false);
     });
     QObject::connect(verifyseed.word16, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word16 + " ");
         verifyseed.word16->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word16->font();
         button.setStrikeOut(true);
         verifyseed.word16->setFont(button);
         verifyseed.word16->setEnabled(false);
     });
     QObject::connect(verifyseed.word17, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word17 + " ");
         verifyseed.word17->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word17->font();
         button.setStrikeOut(true);
         verifyseed.word17->setFont(button);
         verifyseed.word17->setEnabled(false);
     });
     QObject::connect(verifyseed.word18, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word18 + " ");
         verifyseed.word18->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word18->font();
         button.setStrikeOut(true);
         verifyseed.word18->setFont(button);
         verifyseed.word18->setEnabled(false);
     });
        QObject::connect(verifyseed.word19, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word19 + " ");
         verifyseed.word19->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word19->font();
         button.setStrikeOut(true);
         verifyseed.word19->setFont(button);
         verifyseed.word19->setEnabled(false);
     });
     QObject::connect(verifyseed.word20, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word20 + " ");
         verifyseed.word20->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word20->font();
         button.setStrikeOut(true);
         verifyseed.word20->setFont(button);
         verifyseed.word20->setEnabled(false);
     });
     QObject::connect(verifyseed.word21, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word21 + " ");
         verifyseed.word21->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word21->font();
         button.setStrikeOut(true);
         verifyseed.word21->setFont(button);
         verifyseed.word21->setEnabled(false);
     });
     QObject::connect(verifyseed.word22, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word22 + " ");
         verifyseed.word22->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word22->font();
         button.setStrikeOut(true);
         verifyseed.word22->setFont(button);
         verifyseed.word22->setEnabled(false);
     });
     QObject::connect(verifyseed.word23, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word23 + " ");
         verifyseed.word23->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word23->font();
         button.setStrikeOut(true);
         verifyseed.word23->setFont(button);
         verifyseed.word23->setEnabled(false);
     });
     QObject::connect(verifyseed.word24, &QPushButton::clicked, [&] () {

         verifyseed.verify->insertPlainText(word24 + " ");
         verifyseed.word24->setStyleSheet("background-color: rgb(182,182,182);");
         QFont button = verifyseed.word24->font();
         button.setStrikeOut(true);
         verifyseed.word24->setFont(button);
         verifyseed.word24->setEnabled(false);
     });

    dialog.exec();

    if ((verifyseed.verify->toPlainText() == seed) && (verifyseed.verifyBirthday->toPlainText() == birthday))
    {
    char* resp = litelib_execute("save", "");
    QString reply = litelib_process_response(resp);

    auto parsed = json::parse(reply.toStdString().c_str(), nullptr, false);
    if (parsed.is_discarded() || parsed.is_null() || parsed.find("result") == parsed.end()) {

        QMessageBox::warning(this, tr("Failed to save wallet"), 
            tr("Couldn't save the wallet") + "\n" + reply,
            QMessageBox::Ok);
        return false;
    } else {
        return true;
    }
    }else{

        qDebug()<<"Falscher Seed";
        QFile file(dirwalletencfirst);
        QFile file1(dirwalletfirst);

        file.remove();
        file1.remove();
        QMessageBox::warning(this, tr("Wrong Seed"), 
            tr("Please try again") + "\n" ,
            QMessageBox::Ok);
        form.birthday->setVisible(true);
        form.txtSeed->setVisible(true);
        return false;
        this->validatePage();
    }
}


RestoreSeedPage::RestoreSeedPage(FirstTimeWizard *parent) : QWizardPage(parent) {
    this->parent = parent;

    setTitle("Restore wallet from seed");

    QWidget* pageWidget = new QWidget();
    form.setupUi(pageWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pageWidget);
    
    setLayout(layout);
}

bool RestoreSeedPage::validatePage() {
    // 1. Validate that we do have 24 words
    QString seed = form.txtSeed->toPlainText().replace(QRegExp("[ \n\r\t]+"), " ");
    if (seed.trimmed().split(" ").length() != 24) {
        QMessageBox::warning(this, tr("Failed to restore wallet"), 
            tr("SilentDragonLite needs 24 words to restore wallet"),
            QMessageBox::Ok);
        return false;
    }

    // 2. Validate birthday
    QString birthday_str = form.txtBirthday->text();
    bool ok;
    qint64 birthday = birthday_str.toUInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Failed to parse wallet birthday"), 
            tr("Couldn't understand wallet birthday. This should be a block height from where to rescan the wallet. You can leave it as '0' if you don't know what it should be."),
            QMessageBox::Ok);
        return false;
    }

///Number

QString number_str =  form.number->text();
qint64 number = number_str.toUInt();
    // 3. Attempt to restore wallet with the seed phrase
    {
        char* resp = litelib_initialize_new_from_phrase(parent->dangerous, parent->server.toStdString().c_str(),
                seed.toStdString().c_str(), birthday, number);
        QString reply = litelib_process_response(resp);

        if (reply.toUpper().trimmed() != "OK") {
            QMessageBox::warning(this, tr("Failed to restore wallet"), 
                tr("Couldn't restore the wallet") + "\n" + reply,
                QMessageBox::Ok);
            return false;
        } 
    }

    // 4. Finally attempt to save the wallet
    {
        char* resp = litelib_execute("save", "");
        QString reply = litelib_process_response(resp);

        auto parsed = json::parse(reply.toStdString().c_str(), nullptr, false);
        if (parsed.is_discarded() || parsed.is_null() || parsed.find("result") == parsed.end()) {
            QMessageBox::warning(this, tr("Failed to save wallet"), 
                tr("Couldn't save the wallet") + "\n" + reply,
                QMessageBox::Ok);
            return false;
        } else {
            return true;
        }         
    }
}
