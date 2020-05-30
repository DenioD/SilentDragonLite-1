#ifndef CONTACTDATASTORE_H
#define CONTACTDATASTORE_H
#include "../Model/ContactItem.h"
#include <string>
using json = nlohmann::json;

class ContactDataStore
{
    private:
        static bool instanced;
        static ContactDataStore* instance;
        std::map<QString, ContactItem> data;
        ContactDataStore()
        {

        }

    public:
        static ContactDataStore* getInstance();
        void clear();
        void setData(QString key, ContactItem value);
        ContactItem getData(QString key);
        QString dump();

        ~ContactDataStore()
        {
            ContactDataStore::instanced = false;
            ContactDataStore::instance = nullptr;
        }
};



#endif