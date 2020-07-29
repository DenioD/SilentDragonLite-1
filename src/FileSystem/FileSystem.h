// Copyright 2019-2020 The Hush developers
// GPLv3
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QString>
#include <QList>
#include "../Model/ContactItem.h"
#include "../Crypto/FileEncryption.h"
#include <fstream>
using json = nlohmann::json;

class FileSystem
{
    private:
        static bool instanced;
        static FileSystem* instance;
        FileSystem();
        
    public:
        static FileSystem* getInstance();
        QList<ContactItem> readContacts(QString file);
        void writeContacts(QString file, QString data);

        //converter
        QList<ContactItem> readContactsOldFormat(QString file);
        void writeContactsOldFormat(QString file, QList<ContactItem> contacts);
        ~FileSystem();
        
};

#endif
