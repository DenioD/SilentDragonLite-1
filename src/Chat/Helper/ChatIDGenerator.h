#ifndef CHATIDGENERATOR_H
#define CHATIDGENERATOR_H

#include <QString>
#include <QUuid>
#include "../../Model/ChatItem.h"

class ChatIDGenerator
{
    private:
        static ChatIDGenerator* instance;

    public:
        static ChatIDGenerator* getInstance();
        QString generateID(ChatItem item);
};

#endif