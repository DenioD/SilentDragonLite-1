#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "precompiled.h"

#include "logger.h"
#include "recurring.h"

// Forward declare to break circular dependency.
class Controller;
class Settings;
class WSServer;
class WormholeClient;

using json = nlohmann::json;

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

    void replaceWormholeClient(WormholeClient* newClient);
    bool isWebsocketListening();
    void createWebsocket(QString wormholecode);
    void stopWebsocket();

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

    Logger*      logger;

    void doClose();
    QString createHeaderMemo(QString cid, QString zaddr, int version, int headerNumnber);

public slots:
    void slot_change_theme(const QString& themeName);
    void slot_change_currency(const QString& currencyName);
     
private:    
    void closeEvent(QCloseEvent* event);

    void setupSendTab();
    void setupTransactionsTab();
    void setupReceiveTab();
    void setupBalancesTab();
    void setuphushdTab();
    void setupchatTab();
    void updateChat();
    void updateContacts();

    void setupSettingsModal();
    void setupStatusBar();
    
    void clearSendForm();

    Tx   createTxFromSendPage();
    bool confirmTx(Tx tx, RecurringPaymentInfo* rpi);

    Tx   createTxFromChatPage();

    void encryptWallet();
    void removeWalletEncryption();

    void cancelButton();
    void sendButton();
    void sendChatButton();
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

class ChatMemoEdit : public QPlainTextEdit
{
public:
    ChatMemoEdit(QWidget* parent);

    void setMaxLen(int len);
    void setLenDisplayLabel(QLabel* label_40);
    void setSendChatButton(QPushButton* button);
    void includeReplyTo(QString replyToAddress);
    void updateDisplay();

private:
    int             maxlen             = 512;
    QLabel*         lenDisplayLabel    = nullptr;
    QPushButton*    sendChatButton     = nullptr;
};

#endif // MAINWINDOW_H
