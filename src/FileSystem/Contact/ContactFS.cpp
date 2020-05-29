#include "ContactFS.h"

QList<ContactItem> ContactFS::readContacts(QString file)
{
    QList<ContactItem> contacts;   
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
            _file.close();
            ContactFSStrategy* strategy = nullptr; 
            ContactFSContext* context = nullptr;
            qDebug() << version;
            
            if(version == "v1") // we have to convert v2
            {
                strategy = new ContactFSV1AStrategy();
                context = new ContactFSContext(strategy);
                contacts = context->read(file);
                if(contacts.size() > 0)
                {
                     qDebug() << "V1A type address format found";
                     return contacts;
                }
                    
                strategy = new ContactFSV1BStrategy();
                context = new ContactFSContext(strategy);
                contacts = context->read(file);
                if(contacts.size() > 0)
                {
                     qDebug() << "V1B type address format found";
                     return contacts;
                }
                else
                {
                    qDebug() << "No valid V1 type address format found";
                }
    
            }
            else if(version == "v2")
            {
                strategy = new ContactFSV2Strategy();
                context = new ContactFSContext(strategy);
                contacts = context->read(file);
                return contacts;
            }
            else
            {
                qDebug() << "Unknown address type format found";
            }
            

        }

        qDebug() << "Hush contacts readed from disk...";
        
    }
    else
    {
        qDebug() << "No Hush contacts found on disk!";
    }

    return contacts;
}

void ContactFS::write(QString file, QString data)
{

}

