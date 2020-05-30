#ifndef CHAT_H
#define CHAT_H

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

#include "../Model/ChatItem.h"


class Chat // Chat Controller
{
    private:
        QTableView* parent;
        Ui::MainWindow*             ui;
        MainWindow*                 main;
        std::map<QString, QString> cidMap;
        std::map<QString, QString> requestZaddrMap;
    public:
        Chat();
        //QString zaddr();
        void renderChatBox(Ui::MainWindow* ui, QListView *view, QLabel *label); // action
       // void renderContactRequest();
        /*void triggerRequest();
        void showMessages();
        void clear();
        //void renderContactRequest(Ui::MainWindow* ui, QListView *view);
        void addMessage(ChatItem item);
        void addMessage(QString timestamp, ChatItem item);
        void addCid(QString tx, QString cid);
        void addrequestZaddr(QString tx, QString requestZaddr);
        QString getCidByTx(QString tx);
        QString getrequestZaddrByTx(QString tx);
        void killCidCache();
        void killrequestZaddrCache();*/

};

#endif