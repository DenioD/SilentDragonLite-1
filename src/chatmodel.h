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
#include "Chat/Helper/ChatDelegator.h"
#include "Chat/Helper/ChatIDGenerator.h"

class ChatModel
{
    private:
        std::map<QString, ChatItem> chatItems; 
        QTableView* parent;
        Ui::MainWindow*             ui;
        MainWindow*                 main;
        std::map<QString, QString> cidMap;
        std::map<QString, QString> requestZaddrMap;

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
        void addMessage(ChatItem item);
        void addMessage(QString timestamp, ChatItem item);
        void addCid(QString tx, QString cid);
        void addrequestZaddr(QString tx, QString requestZaddr);
        QString getCidByTx(QString tx);
        QString getrequestZaddrByTx(QString tx);
        void killCidCache();
        void killrequestZaddrCache();
        
};

#endif