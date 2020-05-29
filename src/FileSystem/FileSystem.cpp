// Copyright 2019-2020 The Hush developers
// GPLv3

#include "FileSystem.h"
#include <QString>
#include <QList>
#include "../Crypto/passwd.h"
#include "addressbook.h"
#include "./Contact/ContactFS.h"

FileSystem::FileSystem()
{
}

FileSystem* FileSystem::getInstance()
{
    if(!FileSystem::instanced)
    {
        FileSystem::instanced = true;
        FileSystem::instance = new FileSystem();
        FileEncryption::showConfig();
    }
                
    return FileSystem::instance;
}

QList<ContactItem> FileSystem::readContacts(QString file)
{
    return this->readContactsOldFormat(file); //will be called if addresses are in the old dat-format

    QFile _file(file);
    if (_file.exists()) 
    {
        std::ifstream f(file.toStdString().c_str(), std::ios::binary);
        if(f.is_open())
        {
            std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(f), {});
            //todo covert to string to use is as json to feed the data store in addressbook
        }

        f.close();
    }
    else
    {
        qInfo() << file << "not exist";
    }
}

void FileSystem::writeContacts(QString file, QString data)
{
    qDebug() << data;
    QFile _file(file);
    if (_file.exists()) 
    {
        std::ofstream f(file.toStdString().c_str());
        if(f.is_open())
        {
            //ENCRYPT HERE

            f << data.toStdString();
        }

        f.close();
    }
    else
    {
        qInfo() << file << "not exist";
    }    
}

void FileSystem::writeContactsOldFormat(QString file, QList<ContactItem> contacts)
{
    QFile _file(file);
    _file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDataStream out(&_file);   // we will serialize the data into the file
    QList<QList<QString>> _contacts;
    for(auto &item: contacts)
    {
        QList<QString> c;
        c.push_back(item.getName());
        c.push_back(item.getPartnerAddress());
        c.push_back(item.getMyAddress());
        c.push_back(item.getCid());
        c.push_back(item.getAvatar());
        _contacts.push_back(c);
    }
    out << QString("v2") << _contacts;
    _file.close();
}

QList<ContactItem> FileSystem::readContactsOldFormat(QString file)
{
    return ContactFS::readContacts(file);
    /*QList<ContactItem> contacts;
    QFile _file(file);
    if (_file.exists()) 
    {
        contacts.clear();
        _file.open(QIODevice::ReadOnly);
        QDataStream in(&_file);    // read the data serialized from the file
        if(in.status() == QDataStream::ReadCorruptData)
        {
            qDebug() << "Error reading contacts! ---> Your hush contacts from disk maybe corrupted";
            QFile::rename(file, file +  QString(".corrupted"));
            QMessageBox::critical(
                    nullptr, 
                    QObject::tr("Error reading contacts!"), 
                    QObject::tr("Your hush contacts from disk maybe corrupted"), 
                    QMessageBox::Ok
                );
        }
        else    
        {
            QString version;
            in >> version;
            if(version == "v1")
            {
                qDebug() << "Detected old addressbook format";
                // Convert old addressbook format v1 to v2
                QList<QPair<QString,QString>> stuff;
                in >> stuff;
                qDebug() << "Stuff: " << stuff;
                for (int i=0; i < stuff.size(); i++) 
                {
                    ContactItem contact = ContactItem(stuff[i].first, stuff[i].second);
                    contacts.push_back(contact);
                    qDebug() << "contact=" << contact.toQTString();
                }
                
            }
            else
            {
                qDebug() << "Read " << version << " Hush contacts from disk...";
                QList<QList<QString>> stuff;
                in >> stuff;
                qDebug() << "Dataarray size: " << stuff.size();
                if(stuff.size() == 0)
                    return contacts;

                for (int i= 0; i < stuff.size(); i++) 
                {
                    qDebug() << stuff[i].size();
                    ContactItem contact;
                    if(stuff[i].size() == 4)
                    {
                        contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3]);
                    }
                    else
                    {
                        contact = ContactItem(stuff[i][0],stuff[i][1], stuff[i][2], stuff[i][3],stuff[i][4]);
                    }
                    
                    qDebug() << contact.toQTString();
                    contacts.push_back(contact);
                }

                
            }

            qDebug() << "Hush contacts readed from disk...";
        }
            
        _file.close();
    }
    else
    { 
        qDebug() << "No Hush contacts found on disk!";
    }

    return contacts;*/
}

bool FileSystem::fileExists(QString file)
{
    QFileInfo info(file);
    return info.exists() && info.isFile();
}

bool FileSystem::pathExists(QString path)
{
    QFileInfo info(path);
    return info.exists() && info.isDir();
}

FileSystem::~FileSystem()
{
    this->instance = nullptr;
    this->instanced = false;
    delete this->instance;
}

FileSystem *FileSystem::instance = nullptr;
bool FileSystem::instanced = false;