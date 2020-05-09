#ifndef CHAT_H
#define CHAT_H

class Chat // Chat Controller
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
        QString generateChatItemID(ChatItem item); // helper
        std::map<QString, ChatItem> getItems();
        void setItems(std::map<QString, ChatItem> items);
        QString zaddr();
        void setItems(std::vector<ChatItem> items);
        void renderChatBox(Ui::MainWindow* ui, QListView &view); // action
        void renderChatBox(Ui::MainWindow* ui, QListView *view); // action
       // void renderContactRequest();
        void triggerRequest();
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
        void killrequestZaddrCache();

};

#endif