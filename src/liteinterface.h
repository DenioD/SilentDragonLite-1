#ifndef hushDRPC_H
#define hushDRPC_H

#include "precompiled.h"

#include "camount.h"
#include "connection.h"

// Since each transaction can contain multiple outputs, we separate them out individually
// into a struct with address, amount, memo
struct TransactionItemDetail {
    QString         address;
    CAmount         amount;
    QString         memo;
};

// Represents a row in the transactions table. Note that each transaction can contain
// multiple addresses (i.e., Multiple TransctionItemDetail)
struct TransactionItem {
    QString         type;
    qint64          datetime;
    QString         address;
    QString         txid;
    long            confirmations;

    QList<TransactionItemDetail> items;
};


class LiteInterface {
public:
    LiteInterface();
    ~LiteInterface();

    bool haveConnection();
    void setConnection(Connection* c);
    Connection* getConnection() { return conn; }

    void fetchUnspent             (const std::function<void(QJsonValue)>& cb);
    void fetchTransactions        (const std::function<void(QJsonValue)>& cb);
    void fetchAddresses           (const std::function<void(QJsonValue)>& cb);

    void fetchInfo(const std::function<void(QJsonValue)>& cb,
                    const std::function<void(QString)>& err);



    void fetchLatestBlock(const std::function<void(QJsonValue)>& cb,
                        const std::function<void(QString)>& err);
    
    void fetchBalance(const std::function<void(QJsonValue)>& cb);
    

    void createNewZaddr(bool sapling, const std::function<void(QJsonValue)>& cb);
    void createNewTaddr(const std::function<void(QJsonValue)>& cb);
    void createNewSietchZaddr(const std::function<void(QJsonValue)>& cb);

    void fetchPrivKey(QString addr, const std::function<void(QJsonValue)>& cb);
    void fetchAllPrivKeys(const std::function<void(QJsonValue)>);
    void fetchSeed(const std::function<void(QJsonValue)>&);

    void saveWallet(const std::function<void(QJsonValue)>& cb);
    void clearWallet(const std::function<void(QJsonValue)>& cb);

    void fetchWalletEncryptionStatus(const std::function<void(QJsonValue)>& cb);
    void fetchSupply(const std::function<void(QJsonValue)>& cb);
    void encryptWallet(QString password, const std::function<void(QJsonValue)>& cb);
    void unlockWallet(QString password, const std::function<void(QJsonValue)>& cb);
    void removeWalletEncryption(QString password, const std::function<void(QJsonValue)>& cb);

    //void importZPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    //void importTPrivKey(QString addr, bool rescan, const std::function<void(json)>& cb);
    
   
    void sendTransaction(QString params, const std::function<void(QJsonValue)>& cb, const std::function<void(QString)>& err);

private:
    Connection*  conn                        = nullptr;
   
};

#endif // hushDRPC_H
