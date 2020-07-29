#ifndef SETTINGS_H
#define SETTINGS_H

#include "precompiled.h"
#include "camount.h"

using json = nlohmann::json;

struct Config {
    QString server;
};

struct ToFields;
struct Tx;

struct PaymentURI {
    QString addr;
    QString amt;
    QString memo;

    // Any errors are stored here
    QString error;
};

class Settings
{
public:
    static  Settings* init();
    static  Settings* getInstance();

    Config  getSettings();
    void    saveSettings(const QString& server);

    bool    isTestnet();
    void    setTestnet(bool isTestnet);
            
    bool    isSaplingAddress(QString addr);
    bool    isSproutAddress(QString addr);
            
    bool    isValidSaplingPrivateKey(QString pk);

    bool    isSyncing();
    void    setSyncing(bool syncing);

    QString gethushdVersion();
    void    sethushdVersion(QString version);
    
    void    setUseEmbedded(bool r) { _useEmbedded = r; }
    bool    useEmbedded() { return _useEmbedded; }

    void    setHeadless(bool h) { _headless = h; }
    bool    isHeadless() { return _headless; }

    int     getBlockNumber();
    void    setBlockNumber(int number);
            
    bool    getAllowFetchPrices();
    void    setAllowFetchPrices(bool allow);

    bool    getCheckForUpdates();
    void    setCheckForUpdates(bool allow);

    QString get_theme_name();
    void set_theme_name(QString theme_name);

    QString get_currency_name();
    void set_currency_name(QString currency_name);


    bool    isSaplingActive();

    void    setZECPrice(double p) { ZECPrice = p; }
    void    setEURPrice(double p) { EURPrice = p; }
    void    setBTCPrice(double p) { BTCPrice = p; }
    void    setCNYPrice(double p) { CNYPrice = p; }
    void    setRUBPrice(double p) { RUBPrice = p; }
    void    setCADPrice(double p) { CADPrice = p; }
    void    setSGDPrice(double p) { SGDPrice = p; }
    void    setCHFPrice(double p) { CHFPrice = p; }
    void    setINRPrice(double p) { INRPrice = p; }
    void    setGBPPrice(double p) { GBPPrice = p; }
    void    setAUDPrice(double p) { AUDPrice = p; }
    void    setUSDVolume(double p) { USDVolume = p; }
    void    setEURVolume(double p) { EURVolume = p; }
    void    setBTCVolume(double p) { BTCVolume = p; }
    void    setCNYVolume(double p) { CNYVolume = p; }
    void    setRUBVolume(double p) { RUBVolume = p; }
    void    setCADVolume(double p) { CADVolume = p; }
    void    setSGDVolume(double p) { SGDVolume = p; }
    void    setCHFVolume(double p) { CHFVolume = p; }
    void    setINRVolume(double p) { INRVolume = p; }
    void    setGBPVolume(double p) { GBPVolume = p; }
    void    setAUDVolume(double p) { AUDVolume = p; }
    void    setUSDCAP(double p) { USDCAP = p; }
    void    setEURCAP(double p) { EURCAP = p; }
    void    setBTCCAP(double p) { BTCCAP = p; }
    void    setCNYCAP(double p) { CNYCAP = p; }
    void    setRUBCAP(double p) { RUBCAP = p; }
    void    setCADCAP(double p) { CADCAP = p; }
    void    setSGDCAP(double p) { SGDCAP = p; }
    void    setCHFCAP(double p) { CHFCAP = p; }
    void    setINRCAP(double p) { INRCAP = p; }
    void    setGBPCAP(double p) { GBPCAP = p; }
    void    setAUDCAP(double p) { AUDCAP = p; }
    double  getZECPrice();
    double  getEURPrice();
    double  getBTCPrice();
    double  getCNYPrice();
    double  getRUBPrice();
    double  getCADPrice();
    double  getSGDPrice();
    double  getCHFPrice();
    double  getINRPrice();
    double  getGBPPrice();
    double  getAUDPrice();
    double  getUSDVolume();
    double  getEURVolume();
    double  getBTCVolume();
    double  getCNYVolume();
    double  getRUBVolume();
    double  getCADVolume();
    double  getSGDVolume();
    double  getCHFVolume();
    double  getINRVolume();
    double  getGBPVolume();
    double  getAUDVolume();
    double  getUSDCAP();
    double  getEURCAP();
    double  getBTCCAP();
    double  getCNYCAP();
    double  getRUBCAP();
    double  getCADCAP();
    double  getSGDCAP();
    double  getCHFCAP();
    double  getINRCAP();
    double  getGBPCAP();
    double  getAUDCAP();
    

    // Static stuff
    static const QString txidStatusMessage;
    
    static void saveRestore(QDialog* d);
    static void saveRestoreTableHeader(QTableView* table, QDialog* d, QString tablename) ;

    static void openAddressInExplorer(QString address);
    static void openTxInExplorer(QString txid);

    static PaymentURI parseURI(QString paymentURI);
    static QString    paymentURIPretty(PaymentURI);

    static bool    isZAddress(QString addr);
    static bool    isTAddress(QString addr);

    static QString getTokenName();
    static QString getDonationAddr();

    static QString getDefaultServer();
    static CAmount getMinerFee();

    static int     getMaxMobileAppTxns() { return 30; }

    static int     getNumberOfDecimalPlaces() {return 8;}
    
    static bool    isValidAddress(QString addr);

    static QString getDefaultChainName() { return QString("main"); }

    static const QString labelRegExp;

    static const int     updateSpeed         = 30 * 1000;        // 30 sec
    static const int     quickUpdateSpeed    = 5  * 1000;        // 3 sec
    static const int     priceRefreshSpeed   = 5 * 60 * 1000;   // 15 min

private:
    // This class can only be accessed through Settings::getInstance()
    Settings() = default;
    ~Settings() = default;

    static Settings* instance;

    QString _executable;
    bool    _isTestnet        = false;
    bool    _isSyncing        = false;
    int     _blockNumber      = 0;
    QString     _hushdVersion    = 0;
    bool    _useEmbedded      = false;
    bool    _headless         = false;
    
    double  ZECPrice          = 0.0;
    double  BTCPrice          = 0.0;
    double  EURPrice          = 0.0;
    double  CNYPrice          = 0.0;
    double  RUBPrice          = 0.0;
    double  CADPrice          = 0.0;
    double  SGDPrice          = 0.0;
    double  CHFPrice          = 0.0;
    double  INRPrice          = 0.0;
    double  GBPPrice          = 0.0;
    double  AUDPrice          = 0.0;
    double  USDVolume          = 0.0;
    double  EURVolume          = 0.0;
    double  BTCVolume          = 0.0;
    double  CNYVolume          = 0.0;
    double  RUBVolume          = 0.0;
    double  CADVolume          = 0.0;
    double  SGDVolume          = 0.0;
    double  CHFVolume          = 0.0;
    double  INRVolume          = 0.0;
    double  GBPVolume          = 0.0;
    double  AUDVolume          = 0.0;
    double  USDCAP          = 0.0;
    double  EURCAP          = 0.0;
    double  BTCCAP          = 0.0;
    double  CNYCAP          = 0.0;
    double  RUBCAP          = 0.0;
    double  CADCAP          = 0.0;
    double  SGDCAP          = 0.0;
    double  CHFCAP          = 0.0;
    double  INRCAP          = 0.0;
    double  GBPCAP          = 0.0;
    double  AUDCAP          = 0.0;
};


inline bool isJsonResultSuccess(const json& res) {
    return res.find("result") != res.end() && 
                    QString::fromStdString(res["result"].get<json::string_t>()) == "success";
}

inline bool isJsonError(const json& res) {
    return res.find("error") != res.end();
}


#endif // SETTINGS_H
