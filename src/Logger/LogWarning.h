#ifndef LOGWARNING_H
#define LOGWARNING_H

#include "LogType.h"
#include "LogStrategy.h"
#include "LogWriter.h"

class LogWarning : public LogStrategy
{
    public:
        void log(std::string message)
        {
            LogWriter* lw = LogWriter::getInstance();
            lw->write(LogType::WARNING, message);
        }
};

#endif