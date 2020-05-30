#ifndef LOGINFO_H
#define LOGINFO_H

#include "LogType.h"
#include "LogStrategy.h"
#include "LogWriter.h"

class LogInfo : public LogStrategy
{
    public:
        void log(std::string message)
        {
            LogWriter* lw = LogWriter::getInstance();
            lw->write(LogType::INFO, message);
        }
};

#endif