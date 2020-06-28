#include "connection.h"
#include "mainwindow.h"
#include "settings.h"
#include "ui_connection.h"
#include "firsttimewizard.h"
#include "ui_createhushconfdialog.h"
#include "controller.h"
#include "../lib/silentdragonlitelib.h"
#include "precompiled.h"

#ifdef Q_OS_WIN
auto dirwalletconnection = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
#endif
#ifdef Q_OS_MACOS
auto dirwalletconnection = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("silentdragonlite/silentdragonlite-wallet.dat");
#endif
#ifdef Q_OS_LINUX
auto dirwalletconnection = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(".silentdragonlite/silentdragonlite-wallet.dat");
#endif

ConnectionLoader::ConnectionLoader(MainWindow* main, Controller* rpc)
{
    this->main = main;
    this->rpc  = rpc;

    d = new QDialog(main);
    connD = new Ui_ConnectionDialog();
    connD->setupUi(d);

    auto theme = Settings::getInstance()->get_theme_name();
    qDebug() << theme << "theme has loaded";
    auto size  = QSize(512,512);

    if (theme == "Dark" || theme == "Midnight") {
        QMovie *movie2 = new QMovie(":/img/res/silentdragonlite-animated-startup-dark.gif");;
        movie2->setScaledSize(size);
        qDebug() << "Animation dark loaded";
        connD->topIcon->setMovie(movie2);
        movie2->start();
    } else {
        QMovie *movie1 = new QMovie(":/img/res/silentdragonlite-animated-startup.gif");;
        movie1->setScaledSize(size);
        qDebug() << "Animation light loaded";
        connD->topIcon->setMovie(movie1);
        movie1->start();
    }
    main->logger->write("Set animation");
    qDebug() << "Set animation";
    isSyncing = new QAtomicInteger<bool>();
}

ConnectionLoader::~ConnectionLoader()
{
    delete isSyncing;
    delete connD;
    delete d;
}

void ConnectionLoader::loadConnection()
{
    QTimer::singleShot(1, [=]() { this->doAutoConnect(); });
    if (!Settings::getInstance()->isHeadless())
        d->exec();
}

void ConnectionLoader::doAutoConnect()
{
    qDebug() << "Doing autoconnect";
    auto config = std::shared_ptr<ConnectionConfig>(new ConnectionConfig());
    config->server = Settings::getInstance()->getSettings().server;

    // Initialize the library
    main->logger->write(QObject::tr("Attempting to initialize library with ") + config->server);

    // Check to see if there's an existing wallet
    if (litelib_wallet_exists(Settings::getDefaultChainName().toStdString().c_str()))
    {
        main->logger->write(QObject::tr("Using existing wallet."));
        char* resp = litelib_initialize_existing(
            config->server.toStdString().c_str()
        );
        QString response = litelib_process_response(resp);

        if (response.toUpper().trimmed() != "OK")
        {
            showError(response);
            return;
        }

    }
    else
    {
        main->logger->write(QObject::tr("Create/restore wallet."));
        createOrRestore(config->server);
        d->show();
    }

    auto connection = makeConnection(config);
    auto me = this;

    // After the lib is initialized, try to do get info
    connection->doRPC("info", "", [=](QJsonValue reply) {
        // If success, set the connection
        main->logger->write("Connection is online.");
        connection->setInfo(reply);
        main->logger->write("getting Connection reply");
        isSyncing = new QAtomicInteger<bool>();
        isSyncing->storeRelaxed(true);
        main->logger->write("isSyncing");

        // Do a sync at startup
        syncTimer = new QTimer(main);
        main->logger->write("Beginning sync");
        connection->doRPCWithDefaultErrorHandling("sync", "", [=](QJsonValue) {
            isSyncing->storeRelaxed(false);
            // Cancel the timer
            syncTimer->deleteLater();
            // When sync is done, set the connection
            this->doRPCSetConnection(connection);
        });

        // While it is syncing, we'll show the status updates while it is alive.
        QObject::connect(syncTimer, &QTimer::timeout, [=]() {
            // Check the sync status
            if (isSyncing != nullptr && isSyncing->loadRelaxed()) {
                // Get the sync status

                try {
                connection->doRPC("syncstatus", "", [=](QJsonValue reply) {
                   
                    if (isSyncing != nullptr && reply.toObject()["synced_blocks"].toInt())
                    {
                        qint64 synced = reply["synced_blocks"].toInt();
                        qint64 total = reply["total_blocks"].toInt();
                        me->showInformation(
                            "Synced " + QString::number(synced) + " / " + QString::number(total)
                        );
                    }
                },
                [=](QString err) {
                    qDebug() << "Sync error" << err;
                });
            }catch (...)
            {
                main->logger->write("catch sync progress reply");

            }

            }
        });

        syncTimer->setInterval(1* 1000);
        syncTimer->start();
        main->logger->write("Start sync timer");

    }, [=](QString err) {
        showError(err);
    });
}

void ConnectionLoader::createOrRestore(QString server)
{
    // Close the startup dialog, since we'll be showing the wizard
    d->hide();
    // Create a wizard
    FirstTimeWizard wizard(server);
    main->logger->write("Start new Wallet with FirstimeWizard");
    wizard.exec();
}

void ConnectionLoader::doRPCSetConnection(Connection* conn)
{
    qDebug() << "Connectionloader finished, setting connection";
    main->logger->write("Connectionloader finished, setting connection");
    rpc->setConnection(conn);
    d->accept();
    QTimer::singleShot(1, [=]() { delete this; });

try
{

    QFile plaintextWallet(dirwalletconnection);
    main->logger->write("Path to Wallet.dat : " );
    plaintextWallet.remove();

}catch (...)

{

    main->logger->write("no Plaintext wallet.dat");
}
    
}

Connection* ConnectionLoader::makeConnection(std::shared_ptr<ConnectionConfig> config)
{
    return new Connection(main, config);
}

// Update the UI with the status
void ConnectionLoader::showInformation(QString info, QString detail)
{
    connD->status->setText(info);
    connD->statusDetail->setText(detail);
}

/**
 * Show error will close the loading dialog and show an error.
*/
void ConnectionLoader::showError(QString explanation)
{
    rpc->noConnection();
    QMessageBox::critical(
        main,
        QObject::tr("Connection Error"),
        explanation,
        QMessageBox::Ok
    );
    d->close();
}

QString litelib_process_response(char* resp)
{
    char* resp_copy = new char[strlen(resp) + 1];
    //a safer version of strcpy
    strncpy(resp_copy, resp, strlen(resp)+1);
    litelib_rust_free_string(resp);
    QString reply = QString::fromStdString(resp_copy);
    memset(resp_copy, '-', strlen(resp_copy));
    delete[] resp_copy;
    return reply;
}

/***********************************************************************************
 *  Connection, Executor and Callback Class
 ************************************************************************************/
void Executor::run()
{
    char* resp = litelib_execute(this->cmd.toStdString().c_str(), this->args.toStdString().c_str());
    QString reply = litelib_process_response(resp);
    QJsonDocument parsed = QJsonDocument::fromJson(reply.toUtf8());

    if (parsed.isEmpty() || parsed.isNull())
        emit handleError(reply);

    else
    {
        QJsonValue retval;

        if (parsed.isObject())
            retval = QJsonValue(parsed.object());
        else if (parsed.isArray())
            retval = QJsonValue(parsed.array());

        emit responseReady(retval);
     }
}


void Callback::processRPCCallback(QJsonValue resp)
{
    this->cb(resp);
    // Destroy self
    delete this;
}

void Callback::processError(QString resp)
{
    this->errCb(resp);
    // Destroy self
    delete this;
}

Connection::Connection(MainWindow* m, std::shared_ptr<ConnectionConfig> conf)
{
    this->config      = conf;
    this->main        = m;
    // Register the JSON type as a type that can be passed between signals and slots.
    qRegisterMetaType<QJsonValue>("QJsonValue");
}

void Connection::doRPC(const QString cmd, const QString args, const std::function<void(QJsonValue)>& cb, const std::function<void(QString)>& errCb)
{
    if (shutdownInProgress)
        // Ignoring RPC because shutdown in progress
        return;

    //qDebug() << "Doing RPC: " << cmd;

    // Create a runner.
    auto runner = new Executor(cmd, args);

    // Callback object. Will delete itself
    auto c = new Callback(cb, errCb);

    QObject::connect(runner, &Executor::responseReady, c, &Callback::processRPCCallback);
    QObject::connect(runner, &Executor::handleError, c, &Callback::processError);
    QThreadPool::globalInstance()->start(runner);
}

void Connection::doRPCWithDefaultErrorHandling(const QString cmd, const QString args, const std::function<void(QJsonValue)>& cb)
{
    doRPC(cmd, args, cb, [=] (QString err) {
        this->showTxError(err);
    });
}

void Connection::doRPCIgnoreError(const QString cmd, const QString args, const std::function<void(QJsonValue)>& cb)
{
    doRPC(cmd, args, cb, [=] (auto) {
        // Ignored error handling
    });
}

void Connection::showTxError(const QString& error)
{
    if (error.isNull())
        return;

    // Prevent multiple dialog boxes from showing, because they're all called async
    static bool shown = false;
    if (shown)
        return;

    shown = true;
    QMessageBox::critical(
        main,
        QObject::tr("Transaction Error"),
        QObject::tr("There was an error sending the transaction. The error was:") + "\n\n" + error,
        QMessageBox::StandardButton::Ok
    );
    shown = false;
}

/**
 * Prevent all future calls from going through
 */
void Connection::shutdown()
{
    shutdownInProgress = true;
}
