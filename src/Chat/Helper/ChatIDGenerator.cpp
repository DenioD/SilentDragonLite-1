#include "ChatIDGenerator.h"

ChatIDGenerator* ChatIDGenerator::getInstance()
{
    if(ChatIDGenerator::instance == nullptr)
        ChatIDGenerator::instance = new ChatIDGenerator();

    return ChatIDGenerator::instance;
}

QString ChatIDGenerator::generateID(ChatItem item)
{
    QString key = QString::number(item.getTimestamp()) + QString("-");
    key += QString(
        QCryptographicHash::hash(
            QString(
                QString::number(item.getTimestamp()) +
                item.getAddress() +
                item.getContact() +
                item.getMemo()
            ).toUtf8(),
            QCryptographicHash::Md5
        ).toHex()
    );
    return key;
}

ChatIDGenerator* ChatIDGenerator::instance = nullptr;