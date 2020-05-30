#ifndef CHATDATASTORE_H
#define CHATDATASTORE_H
#include "../chatmodel.h"
using json = nlohmann::json;

class ChatDataStore
{
    private:
        static bool instanced;
        static ChatDataStore* instance;
        Ui::MainWindow*             ui;
        MainWindow*                 main;
        std::map<QString, ChatItem> data;
        ChatDataStore()
        {

        }

    public:
        static ChatDataStore* getInstance();
        void clear();
        void setData(QString key, ChatItem value);
        ChatItem getData(QString key);
        std::map<QString, ChatItem> getAllRawChatItems();
        std::map<QString, ChatItem> getAllNewContactRequests();
        std::map<QString, ChatItem> getAllOldContactRequests();
        std::map<QString, ChatItem> getAllMemos();
        QString dump();

        ~ChatDataStore()
        {
            ChatDataStore::instanced = false;
            ChatDataStore::instance = nullptr;
        }
};



#endif