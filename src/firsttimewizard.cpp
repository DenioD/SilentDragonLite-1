#include "firsttimewizard.h"

#include "ui_newseed.h"
#include "ui_restoreseed.h"
#include "ui_newwallet.h"
#include "mainwindow.h"
#include "DataStore/DataStore.h"

#include "../lib/silentdragonlitelib.h"


FirstTimeWizard::FirstTimeWizard(QString server)
{
    setWindowTitle("New wallet wizard");
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
             parent->button(QWizard::CommitButton)->setEnabled(true);

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
         //main->setPassword(Password);

         //qDebug()<<"Objekt gesetzt";
            
 
                // Exclusive buttons
    QObject::connect(form.radioNewWallet,  &QRadioButton::clicked, [=](bool checked) {
        if (checked) {
            form.radioRestoreWallet->setChecked(false);
            
        }
    });

    QObject::connect(form.radioRestoreWallet, &QRadioButton::clicked, [=](bool checked) {
        if (checked) {
            form.radioNewWallet->setChecked(false);
          
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

    char* resp = litelib_initialize_new(parent->server.toStdString().c_str());
    QString reply = litelib_process_response(resp);

    QByteArray ba_reply = reply.toUtf8();
    QJsonDocument jd_reply = QJsonDocument::fromJson(ba_reply);
    QJsonObject parsed = jd_reply.object();

    if (parsed.isEmpty() || parsed["seed"].isNull()) {
        form.txtSeed->setPlainText(tr("Error creating a wallet") + "\n" + reply);
    } else {
        QString seed = parsed["seed"].toString();
        form.txtSeed->setPlainText(seed);
    }


}

// Will be called just before closing. Make sure we can save the seed in the wallet
// before we allow the page to be closed
bool NewSeedPage::validatePage() {
    char* resp = litelib_execute("save", "");
    QString reply = litelib_process_response(resp);

    QByteArray ba_reply = reply.toUtf8();
    QJsonDocument jd_reply = QJsonDocument::fromJson(ba_reply);
    QJsonObject parsed = jd_reply.object();

    if (parsed.isEmpty() || parsed["result"].isNull()) {
        QMessageBox::warning(this, tr("Failed to save wallet"), 
            tr("Couldn't save the wallet") + "\n" + reply,
            QMessageBox::Ok);
        return false;
    } else {
        return true;
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
        char* resp = litelib_initialize_new_from_phrase(parent->server.toStdString().c_str(),
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

        QByteArray ba_reply = reply.toUtf8();
        QJsonDocument jd_reply = QJsonDocument::fromJson(ba_reply);
        QJsonObject parsed = jd_reply.object();

        if (parsed.isEmpty() || parsed["result"].isNull()) {
            QMessageBox::warning(this, tr("Failed to save wallet"), 
                tr("Couldn't save the wallet") + "\n" + reply,
                QMessageBox::Ok);
            return false;
        } else {
            return true;
        }         
    }
}
