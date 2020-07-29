// Copyright 2019-2020 The Hush developers
// GPLv3

#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include "precompiled.h"

#include "camount.h"
#include "datamodel.h"
#include "balancestablemodel.h"
#include "txtablemodel.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "liteinterface.h"
#include "connection.h"
#include "chatmodel.h"
#include "Chat/Chat.h"
#include "Model/ContactRequestChatItem.h"
#include "Model/ContactItem.h"
#include "contactmodel.h"

using json = nlohmann::json;

struct WatchedTx {
    QString opid;
    Tx tx;
    std::function<void(QString, QString)> completed;
    std::function<void(QString, QString)> error;
};


class Controller
{
public:
    Controller(MainWindow* main);
    ~Controller();

    DataModel* getModel() { return model; }

    Connection* getConnection() { return zrpc->getConnection(); }
    void setConnection(Connection* c);
    void refresh(bool force = false);
    void refreshAddresses();
    int getLag();   
    void setLag(int lag);
    int _lag;
    std::string encryptDecrypt(std::string);
    
    void checkForUpdate(bool silent = true);
    void refreshZECPrice();
    void refreshEURPrice();
    void refreshBTCPrice();
    void refreshCNYPrice();
    void refreshRUBPrice();
    void refreshCADPrice();
    void refreshSGDPrice();
    void refreshCHFPrice();
    void refreshINRPrice();
    void refreshGBPPrice();
    void refreshAUDPrice();
    void refreshUSDVolume();
    void refreshEURVolume();
    void refreshBTCVolume();
    void refreshCNYVolume();
    void refreshRUBVolume();
    void refreshCADVolume();
    void refreshSGDVolume();
    void refreshCHFVolume();
    void refreshINRVolume();
    void refreshGBPVolume();
    void refreshAUDVolume();
    void refreshUSDCAP();
    void refreshEURCAP();
    void refreshBTCCAP();
    void refreshCNYCAP();
    void refreshRUBCAP();
    void refreshCADCAP();
    void refreshSGDCAP();
    void refreshCHFCAP();
    void refreshINRCAP();
    void refreshGBPCAP();
    void refreshAUDCAP();
    
    
    void refreshChat(QListView *listWidget, QLabel *label);
    void refreshContacts(QListView *listWidget);
    
    void executeStandardUITransaction(Tx tx); 

    void executeTransaction(Tx tx, 
        const std::function<void(QString txid)> submitted,
        const std::function<void(QString txid, QString errStr)> error);

    void fillTxJsonParams(json& params, Tx tx);
    
    const TxTableModel*               getTransactionsModel() { return transactionsTableModel; }

    void shutdownhushd();
    void noConnection();
    bool isEmbedded() { return ehushd != nullptr; }

    void encryptWallet(QString password, const std::function<void(json)>& cb) {
        zrpc->encryptWallet(password, cb); 
    }
    
    void removeWalletEncryption(QString password, const std::function<void(json)>& cb) {
                zrpc->removeWalletEncryption(password, cb); }

    void saveWallet(const std::function<void(json)>& cb) { zrpc->saveWallet(cb); }

    void clearWallet(const std::function<void(json)>& cb) { zrpc->clearWallet(cb); }

    void createNewZaddr(bool sapling, const std::function<void(json)>& cb) {
        unlockIfEncrypted([=] () {
            zrpc->createNewZaddr(sapling, cb);
        }, [=](){});
    }

   
    void createNewTaddr(const std::function<void(json)>& cb) {
        unlockIfEncrypted([=] () {
            zrpc->createNewTaddr(cb); 
        }, [=](){});
    }
     void createNewSietchZaddr(const std::function<void(json)>& cb) {
        unlockIfEncrypted([=] () {
           zrpc->createNewSietchZaddr(cb); 
        }, [=](){});
    }
    void fetchPrivKey(QString addr, const std::function<void(json)>& cb) {
        unlockIfEncrypted([=] () {
            zrpc->fetchPrivKey(addr, cb); 
        },
        [=]() {
            cb({ {"error", "Failed to unlock wallet"} });
        });
    }

    void fetchAllPrivKeys(const std::function<void(json)> cb) {
        unlockIfEncrypted([=] () {
            zrpc->fetchAllPrivKeys(cb); 
        },
        [=]() {
            cb({ {"error", "Failed to unlock wallet"} });
        });
    }

    void fetchSeed(const std::function<void(json)> cb) {
        unlockIfEncrypted([=] () {
            zrpc->fetchSeed(cb); 
        },
        [=]() {
            cb({ {"error", "Failed to unlock wallet"} });
        });
    }


    // void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) { zrpc->importZPrivKey(addr, rescan, cb); }
    // void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb) { zrpc->importTPrivKey(addr, rescan, cb); }

    QString getDefaultSaplingAddress();
    QString getDefaultTAddress();   
    
    
private:
    void processInfo(const json&);
    void refreshBalances();

    void refreshTransactions();    

    void processUnspent     (const json& reply, QMap<QString, CAmount>* newBalances, QList<UnspentOutput>* newUnspentOutputs);
    void updateUI           (bool anyUnconfirmed);
    void updateUIBalances   ();

    void getInfoThenRefresh (bool force);

    void unlockIfEncrypted  (std::function<void(void)> cb, std::function<void(void)> error);
    
    
    QProcess*                   ehushd                     = nullptr;

    TxTableModel*               transactionsTableModel      = nullptr;
    BalancesTableModel*         balancesTableModel          = nullptr;

    DataModel*                  model;
    LiteInterface*              zrpc;

    QTimer*                     timer;
    QTimer*                     txTimer;
    QTimer*                     priceTimer;

    Ui::MainWindow*             ui;
    MainWindow*                 main;


    // Current balance in the UI. If this number updates, then refresh the UI
    QString                     currentBalance;
    QString                     sietch;
};


#endif // RPCCLIENT_H
