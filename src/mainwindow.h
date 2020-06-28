#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "precompiled.h"

#include "logger.h"
#include "recurring.h"
#include "firsttimewizard.h"

// Forward declare to break circular dependency.
class Controller;
class Settings;
class WSServer;
class WormholeClient;
class ChatModel;


// Struct used to hold destination info when sending a Tx. 
struct ToFields {
    QString addr;
    CAmount amount;
    QString memo;
};

// Struct used to represent a Transaction. 
struct Tx {
    QString         fromAddr;
    QList<ToFields> toAddrs;
    CAmount         fee;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateLabelsAutoComplete();
    Controller* getRPC() { return rpc; }

    QCompleter*         getLabelCompleter() { return labelCompleter; }
    QRegExpValidator*   getAmountValidator() { return amtValidator; }

    QString doSendTxValidations(Tx tx);
    QString doSendChatTxValidations(Tx tx);
    QString doSendChatMoneyTxValidations(Tx tx);
    QString doSendRequestTxValidations(Tx tx);
    QString doSendChatMoneyRequestTxValidations(Tx tx);
    QString getCid();
    QString getAmt();
    QString getMoneyMemo();
    QString getPassword();
    std::map<QString, QString> pubkeyMap;
    QString getPubkeyByAddress(QString requestZaddr);
    void setPassword(QString Password);
    void setAmt(QString Amt);
    void setMoneyMemo(QString MoneyMemo);
    void addPubkey(QString requestZaddr, QString pubkey);
    
    
    

    void replaceWormholeClient(WormholeClient* newClient);
    bool isWebsocketListening();
    void createWebsocket(QString wormholecode);
    void stopWebsocket();
    void saveContact();
    void saveandsendContact();
    void showRequesthush();
  
    void balancesReady();
    void payhushURI(QString uri = "", QString myAddr = "");

    void updateLabels();
    void updateTAddrCombo(bool checked);
    
    // Disable recurring on mainnet
    void disableRecurring();

    // Check whether the RPC is returned and payments are ready to be made
    bool isPaymentsReady() { return uiPaymentsReady; }

    Ui::MainWindow*     ui;

    QLabel*             statusLabel;
    QLabel*             statusIcon;
    QLabel*             loadingLabel;
    QWidget*            hushdtab;
    //ChatItem* currentChatItem;
    

    Logger*      logger;

    void doClose();
    void doClosePw();
    QString createHeaderMemo(QString type, QString cid, QString zaddr,QString headerbytes,QString publickey, int version, int headerNumber);

public slots:
    void slot_change_theme(const QString& themeName);
    void slot_change_currency(const QString& currencyName);
    
     
private slots:
    

    void on_givemeZaddr_clicked();

    void on_emojiButton_clicked();
private:

    bool fileExists(QString path);
    void closeEvent(QCloseEvent* event);
    void closeEventpw(QCloseEvent* event);
    QString _password;
    QString _amt;
    QString _moneymemo;


    void setupSendTab();
    void setupTransactionsTab();
    void setupReceiveTab();
    void setupBalancesTab();
    void setuphushdTab();
    void setupchatTab();
    void renderContactRequest();
    
    void updateContacts();
    void updateChat();

    void setupSettingsModal();
    void setupStatusBar();
    
    void clearSendForm();
    

    Tx   createTxFromSendPage();
    bool confirmTx(Tx tx, RecurringPaymentInfo* rpi);

    Tx   createTxFromChatPage();
    Tx   createTxForSafeContactRequest();
    Tx   createTxFromSendChatPage();
    Tx   createTxFromSendRequestChatPage();

    void encryptWallet();
    void removeWalletEncryption();
    void removeWalletEncryptionStartUp();

    void cancelButton();
    void sendButton();
    void sendChat();
    void sendMoneyChat();
    void sendMoneyRequestChat();
    void addContact();
    void ContactRequest();
    
    
    void addAddressSection();
    void maxAmountChecked(int checked);

    void editSchedule();

    void addressChanged(int number, const QString& text);
    void amountChanged (int number, const QString& text);

    void addNewZaddr(bool sapling);
    std::function<void(bool)> addZAddrsToComboList(bool sapling);

    void memoButtonClicked(int number, bool includeReplyTo = false);
    void setMemoEnabled(int number, bool enabled);
    
    void donate();
    void discord();
    void website();
    void addressBook();
    void importPrivKey();
    void exportAllKeys();
    void exportKeys(QString addr = "");
    void exportSeed();
    void exportTransactions();

    void doImport(QList<QString>* keys);

    void restoreSavedStates();
    bool eventFilter(QObject *object, QEvent *event);

  


    bool            uiPaymentsReady    = false;
    QString         pendingURIPayment;

    WSServer*       wsserver = nullptr;
    WormholeClient* wormhole = nullptr;


    Controller*         rpc             = nullptr;
    
    QCompleter*         labelCompleter  = nullptr;
    QRegExpValidator*   amtValidator    = nullptr;
    QRegExpValidator*   feesValidator   = nullptr;

    RecurringPaymentInfo* sendTxRecurringInfo = nullptr;

    QMovie*      loadingMovie;
};

class ChatMemoEdit : public QTextEdit
{
public:
    ChatMemoEdit(QWidget* parent);

    void setMaxLenChat(int len);
    void setLenDisplayLabelChat(QLabel* label);
    void SetSendChatButton(QPushButton* button);
    void updateDisplayChat();

private:
    int             maxlenchat             = 235;
    QLabel*         lenDisplayLabelchat    = nullptr;
    QPushButton*    sendChatButton     = nullptr;
};

class ChatMemoEditRequest : public QTextEdit
{
public:
    ChatMemoEditRequest(QWidget* parent);

    void setMaxLenChatRequest(int len);
    void setLenDisplayLabelChatRequest(QLabel* label);
    void SetSendRequestButton(QPushButton* button);
    void updateDisplayChatRequest();

private:
    int             maxlenchatrequest             = 512;
    QLabel*         lenDisplayLabelchatRequest   = nullptr;
    QPushButton*    sendRequestButton     = nullptr;
};


#endif // MAINWINDOW_H
