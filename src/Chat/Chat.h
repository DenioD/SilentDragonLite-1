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
        void renderChatBox(Ui::MainWindow* ui, QListView *view, QLabel *label); // action

};

#endif