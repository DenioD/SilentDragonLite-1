// Copyright 2019-2020 The Hush developers
// GPLv3

#ifndef CHATMODEL_H
#define CHATMODEL_H
#include <QString>
#include <QStandardItemModel>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <map>
#include <vector>
#include <QListView>
#include "precompiled.h"
#include "mainwindow.h"
#include "controller.h"
#include "settings.h"
#include "camount.h"
#include "Model/ChatItem.h"
#include "Model/ContactRequest.h"
#include "Chat/Helper/ChatDelegator.h"
#include "Chat/Helper/ChatIDGenerator.h"


namespace Ui {
    class MainWindow;
}
class ChatModel
{
    private:
        std::map<QString, ChatItem> chatItems; 
        QTableView* parent;
        Ui::MainWindow*             ui;
        MainWindow*                 main;
        std::map<QString, QString> cidMap;
        std::map<QString, QString> requestZaddrMap;
        std::map<QString, QString> confirmationsMap;
        std::map<int, std::tuple<QString, QString, QString>> sendrequestMap;
        std::map<QString, QString> AddressbyLabelMap;

    public:
        ChatModel() {};
        ChatModel(std::map<QString, ChatItem> chatItems);
        ChatModel(std::vector<ChatItem> chatItems);
        std::map<QString, ChatItem> getItems();
        void setItems(std::map<QString, ChatItem> items);
        QString zaddr();
        void setItems(std::vector<ChatItem> items);
        void triggerRequest();
        void showMessages();
        void clear();
        void addAddressbylabel(QString addr, QString label);
        void addMessage(ChatItem item);
        void addMessage(QString timestamp, ChatItem item);
        void addCid(QString tx, QString cid);
        void addrequestZaddr(QString tx, QString requestZaddr);
        void addconfirmations(QString tx, int confirmation);
        void addSendRequest(int i, QString myAddr, QString cid, QString addr );
        QString getCidByTx(QString tx);
        QString getrequestZaddrByTx(QString tx);
        QString getConfirmationByTx(QString tx);
        QString Addressbylabel(QString addr);
        void killCidCache();
        void killConfirmationCache();
        void killrequestZaddrCache();
      
};

#endif